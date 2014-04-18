#include "rpc_server.hpp"

#include <cassert>
#include <iostream>

#include "rpc_event_service.hpp"
#include "rpc_object_manager.hpp"
#include "rpc_controller.hpp"
#include "rpc_service.hpp"


namespace NanoRpc {


class ExternallyControlledLifetimeWrapper : public IRpcService
{
public:
	ExternallyControlledLifetimeWrapper( IRpcService *object ) :
		object_( object )
	{
	}

	virtual void CallMethod( const RpcCall &rpc_call, RpcResult *rpc_result )
	{
		object_->CallMethod( rpc_call, rpc_result );
	}

private:
	IRpcService *object_;
};


RpcServer::RpcServer( RpcController *controller ) :
	controller_( controller )
{
	controller_->set_server( this );
	RegisterService( RpcObjectManager::ServiceName, new ExternallyControlledLifetimeWrapper( &object_manager_ ) );
	RegisterService( RpcEventService::ServiceName, new ExternallyControlledLifetimeWrapper( &event_service_ ) );
}


RpcServer::~RpcServer()
{
	if( controller_ != NULL ) {
		controller_->set_server( NULL );
	}
	
	controller_ = NULL;
}


void RpcServer::RegisterService( const char *name, IRpcService *service )
{
	assert( service != NULL );
	object_manager_.RegisterService( name, service );
}


void RpcServer::RegisterService( IRpcStub *stub )
{
	assert( stub != NULL );
	object_manager_.RegisterService( stub );
}


void RpcServer::Receive( const RpcMessage &rpcMessage )
{
	if( rpcMessage.has_result() && rpcMessage.result().status() != RpcSucceeded ) {
		// TODO: Dont know what to do in this case yet.
		// We were waiting for the message and seen the channel break and received notification about it.
		// The best we can do is to notify the context (not there yet) about broken connection.
		// Currently there are no context and the server does not differentiate between clients and serve them
		// all as one. On the other side the RpcController can act as context.
		// TODO: Probably we should delist the client from event manager.
		return;
	}

	// TODO: Asynchronous calls, see below.
	/*
	// Here we have the option of handling incoming calls sequentually 
	// (basically allowing single thread service) or making them asynchronously
	// thus making the service implementation responsible for handling multithreaded calls.
	// This theoretically can be controlled by the attribute AllowAsynchronousCalls that applies
	// to class and method (in override fashion to the default server behavior).
	// The default server behavior can also be specified as Syncrhonous or Asynchronous.
	 */
	// For now make all calls synchronous.

	IRpcService *service = NULL;

	RpcMessage resultMessage;
	resultMessage.set_id( rpcMessage.id() );

	// Check if the call relates to the singleton or transient object and
	// then try to find the appropriate service.
	//
	// The singleton objects are registered as services and identified by service name. The singleton
	// object's lifetime is controlled by the server.
	// The transient objects are registered as result of a method call and identified by context ID.
	// The lifetime of a transient object controlled by the client.
	if( rpcMessage.call().object_id() != 0 ) {
		// Try to find object that corresponds to the marshalled interface.
		service = object_manager_.GetInstance( rpcMessage.call().object_id() );
		if( service == NULL ) {
			// The requested service was not found. Reply with an error.
			resultMessage.mutable_result()->set_status( RpcUnknownInterface );
			resultMessage.mutable_result()->set_error_message( "Marshalled object does not exist (was object disposed?)." );
		}
	}
	else {
		// Try to find the requested service.
		service = object_manager_.GetService( rpcMessage.call().service().c_str() );
		if( service == NULL ) {
			// The requested service was not found. Reply with an error.
			resultMessage.mutable_result()->set_status( RpcUnknownInterface );
			resultMessage.mutable_result()->set_error_message( "Unknown interface" );
		}
	}

	// If service was found - service the call, otherwise respond with an error.
	if( service != NULL ) {
		RpcResult rpc_result;

		// Call the requested method.
		service->CallMethod( rpcMessage.call(), &rpc_result );
		// TODO: See comment in else branch on expects_result and handling 
		// rpc_result containing an error.
		if( rpcMessage.call().expects_result() ) {
			// Send back the result to the client if client requested it.
			assert( resultMessage.has_id() );
			resultMessage.mutable_result()->MergeFrom( rpc_result );
			controller_->Send( resultMessage );
		}
	}
	else {
		// If client does not expect result, there is no point of sending one,
		// even if it is an error message.
		// TODO: It possibly make sense to implement catch all
		// handler on the client that would handle all reply messages that
		// do not have pending calls. If that's the case, then expects_result
		// condition have to be removed. Also, it make sense control the behavior,
		// whether the server replies with an error message, even if the call does 
		// not expect the result, through an option on the server.
		if( rpcMessage.call().expects_result() ) {
			assert( resultMessage.has_id() );
			assert( resultMessage.has_result() );
			assert( resultMessage.result().has_status() );
			controller_->Send( resultMessage );
		}
	}
}


void RpcServer::Send( RpcMessage &rpcMessage )
{
	assert( rpcMessage.has_call() );
	assert( !rpcMessage.call().expects_result() );

	// Check that client subscribed for event notification.
	if( event_service_.HasInterface( rpcMessage.call().service() ) ) {
		controller_->Send( rpcMessage );
	}
}


} // namespace

