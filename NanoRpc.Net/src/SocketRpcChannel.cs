// <summary>
// </summary>

namespace NanoRpc
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.IO;
    using System.Net.Sockets;

    /// <summary>
    /// An RpcChannel that communicates over a Stream. Does not work with any Stream, it must be a bidrectional channel
    /// like NetworkStream.
    /// </summary>
    public class SocketRpcChannel : RpcChannel
    {
        private readonly Socket _socket;

        private readonly List<SocketAsyncEventArgs> _readEventArgsPool = new List<SocketAsyncEventArgs>();
        private readonly List<SocketAsyncEventArgs> _writeEventArgsPool = new List<SocketAsyncEventArgs>();

        /// <summary>
        /// </summary>
        /// <param name="controller">
        /// </param>
        /// <param name="socket">
        /// </param>
        public SocketRpcChannel(RpcController controller, Socket socket)
            : base(controller)
        {
            _socket = socket;
            IsConnected = true;
        }

        private SocketAsyncEventArgs GetNextReadSocketAsyncEventArgs()
        {
            lock( _readEventArgsPool )
            {
                if( _readEventArgsPool.Count != 0 )
                {
                    var ea1 = _readEventArgsPool[_readEventArgsPool.Count - 1];
                    _readEventArgsPool.RemoveAt(_readEventArgsPool.Count - 1);
                    return ea1;
                }
            }

            var ea = new SocketAsyncEventArgs();
            ea.Completed += HandleReceivedData;
            return ea;
        }

        private SocketAsyncEventArgs GetNextWriteSocketAsyncEventArgs()
        {
            lock( _writeEventArgsPool )
            {
                if( _writeEventArgsPool.Count != 0 )
                {
                    var ea1 = _writeEventArgsPool[_writeEventArgsPool.Count - 1];
                    _writeEventArgsPool.RemoveAt(_writeEventArgsPool.Count - 1);
                    return ea1;
                }
            }

            var ea = new SocketAsyncEventArgs();
            ea.Completed += WriteCompleted;
            return ea;
        }

        /// <summary>
        /// </summary>
        public bool IsConnected { get; set; }

        /// <summary>
        /// </summary>
        public event EventHandler Disconnected;

        /// <summary>
        /// </summary>
        private class ReadOperationState
        {
            private byte[] _buffer = new byte[1024];

            /// <summary>
            /// </summary>
            public ReadOperationState()
            {
                IsWaitingForHeader = true;
            }

            /// <summary>
            /// </summary>
            public bool IsWaitingForHeader { get; set; }

            /// <summary>
            /// </summary>
            public byte[] Buffer
            {
                get { return _buffer; }
            }

            /// <summary>
            /// </summary>
            /// <param name="size">
            /// </param>
            public void EnsureBufferSize(int size)
            {
                if( _buffer.Length < size )
                    _buffer = new byte[size];
            }
        }

        /// <summary>
        /// </summary>
        public override void Start()
        {
            var readOperationState = new ReadOperationState();
            readOperationState.EnsureBufferSize(sizeof( int ));

            var readEventArgs = GetNextReadSocketAsyncEventArgs();
            readEventArgs.UserToken = readOperationState;
            readEventArgs.SetBuffer(readOperationState.Buffer, 0, sizeof( int ));

#if DEBUG_CONSOLE
            Console.WriteLine("Waiting for message prolog");
#endif
            if( !_socket.ReceiveAsync(readEventArgs) )
                HandleReceivedData(this, readEventArgs);
        }

        private void HandleReceivedData(object source, SocketAsyncEventArgs eventArgs)
        {
            if( eventArgs.SocketError == SocketError.Success && eventArgs.BytesTransferred != 0 )
            {
                var readOperationState = eventArgs.UserToken as ReadOperationState;
                if( readOperationState != null )
                {
                    if( readOperationState.IsWaitingForHeader )
                    {
                        readOperationState.IsWaitingForHeader = false;
                        // TODO: For sake of compatibility use protobuf IO to serialize the message size.
                        int expectedPacketSize = BitConverter.ToInt32(eventArgs.Buffer, 0);
                        readOperationState.EnsureBufferSize(expectedPacketSize);
                        eventArgs.SetBuffer(readOperationState.Buffer, 0, expectedPacketSize);

#if DEBUG_CONSOLE
                        Console.WriteLine("Prologue received, expecting message of {0} bytes", expectedPacketSize);
#endif
                    }
                    else
                    {
                        readOperationState.IsWaitingForHeader = true;
                        var memStream = new MemoryStream(eventArgs.Buffer, 0, eventArgs.BytesTransferred);

// TODO: It would be better to parse directly from eventArgs.Buffer, but now the buffer size may be bigger than eventArgs.BytesTransferred
                        RpcMessage message = RpcMessage.ParseFrom(memStream);

#if DEBUG_CONSOLE
                         if(message.Call != null)
                         {
                             Console.WriteLine("Message {0} received, call for {1}::{2}", message.Id,
                             message.Call.Service, message.Call.Method);
                         }
                         else
                         {
                             Console.WriteLine("Reply to message {0} received with status {1}.", message.Id,
                             message.Result.Status);
                         }
#endif
                        Receive(message);
                        readOperationState.EnsureBufferSize(sizeof( int ));
                        eventArgs.SetBuffer(readOperationState.Buffer, 0, sizeof( int ));

#if DEBUG_CONSOLE
                        Console.WriteLine("Waiting for message prolog");
#endif
                    }

                    // TODO: If source is really fast, the we could dig pretty deep into the stack. So, avoid recursion.
#if DEBUG_CONSOLE
                    // Console.WriteLine("Waiting for incoming messages");
#endif
                    // TODO: Handle socket disposed exception, when connection is closed.
                    if( !_socket.ReceiveAsync(eventArgs) )
                        HandleReceivedData(this, eventArgs);
                }
                else
                {
                    Debug.Assert(false, "ReadOperationState is null");
                }
            }
            else
            {
                Console.WriteLine("Connection failure - cancelling pending calls. Reason: {0}", eventArgs.SocketError);
                IsConnected = false;
                RpcMessage failureMessage =
                    new RpcMessage.Builder().SetResult(
                        new RpcResult.Builder().SetStatus(RpcStatus.RpcChannelFailure).SetErrorMessage(
                            "RPC channel failure").Build()).Build();
                Receive(failureMessage);
                SendDisconnectEventAsynchronously();
            }
        }


        /// <summary>
        /// Closes all the streams and requests that the channel shutdown, then joins the channel threads until they
        /// have terminated.
        /// </summary>
        public void CloseAndJoin()
        {
            // readStream must be closed, it's the only way we can terminate the readThread cleanly. Even async reads
            // don't support cancellation
            // socket.Disconnect( true );
            // socket.Close( 2000 );
        }

        /// <summary>
        /// </summary>
        /// <param name="message">
        /// </param>
        internal override void Send(RpcMessage message)
        {
            if( !IsConnected )
            {
                throw new RpcException( RpcStatus.RpcChannelFailure );
            }

            var memStream = new MemoryStream();

            // TODO: For sake of compatibility use protobuf IO to serialize the message size.
            memStream.Write(BitConverter.GetBytes((Int32)message.SerializedSize), 0, sizeof(Int32));
            message.WriteTo(memStream);
            var writeEventArgs = GetNextWriteSocketAsyncEventArgs();
            writeEventArgs.SetBuffer(memStream.GetBuffer(), 0, (int) memStream.Position);

#if DEBUG_CONSOLE
            // Console.WriteLine( "Sending message {0} of {1} bytes", message.Id, args.Buffer.Length );
#endif
            if( !_socket.SendAsync(writeEventArgs) )
                WriteCompleted(this, writeEventArgs);
        }

        private void WriteCompleted(object sender, SocketAsyncEventArgs e)
        {
#if DEBUG_CONSOLE
            // Console.WriteLine("Message sent");
#endif
            lock( _writeEventArgsPool )
            {
                _writeEventArgsPool.Add(e);
            }
        }

        private void SendDisconnectEventAsynchronously()
        {
            if( Disconnected != null )
                Disconnected.EndInvoke(Disconnected.BeginInvoke(this, EventArgs.Empty, null, null));
        }
    }
}