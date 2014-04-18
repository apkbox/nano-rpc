// <summary> 
// </summary>

namespace NanoRpc
{
    using System;
    using System.Diagnostics;
    using System.Net.Sockets;

    /// <summary>
    /// The class asynchronously waits for incoming connections and invokes event
    /// handler when incoming connection accepted. 
    /// </summary>
    public class ServerSocketConnector
    {
        /// <summary>
        /// </summary>
        public ServerSocketConnector()
        {
        }

        /// <summary>
        /// </summary>
        /// <param name="listeningSocket">
        /// </param>
        public ServerSocketConnector(Socket listeningSocket)
        {
            ListeningSocket = listeningSocket;
        }

        /// <summary>
        /// The event fired when new connection accepted.
        /// </summary>
        public event EventHandler<SocketAsyncEventArgs> ConnectionAccepted;

        /// <summary>
        /// </summary>
        public Socket ListeningSocket { get; set; }

        /// <summary>
        /// </summary>
        public void StartAcceptingConnections()
        {
            // TODO: This method must be synchronized.
            // TODO: SocketAsyncEventArgs should be in the pool.
// ReSharper disable PossibleNullReferenceException
            Debug.Assert(ListeningSocket != null, "ListeningSocket property must not be null.");
            Debug.Assert(ListeningSocket.IsBound, "Listening socket must be bound.");
            Debug.Assert(ConnectionAccepted != null, "Connection accepted event handler must be set.");

// ReSharper restore PossibleNullReferenceException
            ListeningSocket.Listen(5);

            var asyncEventArgs = new SocketAsyncEventArgs();
            asyncEventArgs.Completed += ConnectionAcceptedHandler;
            Console.WriteLine("Waiting for connection to {0} ...", ListeningSocket.LocalEndPoint);
            if( !ListeningSocket.AcceptAsync(asyncEventArgs) )
            {
                ConnectionAcceptedHandler(this, asyncEventArgs);
            }
        }

        private static void CloseConnection(SocketAsyncEventArgs e)
        {
            if( e.AcceptSocket.Connected )
            {
                e.AcceptSocket.Shutdown(SocketShutdown.Both);
            }

            e.AcceptSocket.Close(1000);
            e.Dispose();
        }

        private void ConnectionAcceptedHandler(object sender, SocketAsyncEventArgs e)
        {
            if( ConnectionAccepted != null )
            {
                if( sender == this )
                {
                    Console.WriteLine("Connection accepted synchronously.");
                }
                else
                {
                    Console.WriteLine("Connection accepted asynchronously.");
                }

                ConnectionAccepted.EndInvoke(ConnectionAccepted.BeginInvoke(this, e, null, null));
            }
            else
            {
                Console.WriteLine("warning: Accepted connection closed because ConnectionAccepted event was not set.");
                CloseConnection(e);
            }
        }
    }
}

