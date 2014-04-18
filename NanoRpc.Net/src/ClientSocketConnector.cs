namespace NanoRpc
{
    using System;
    using System.Diagnostics;
    using System.Net;
    using System.Net.Sockets;

    /// <summary>
    /// </summary>
    public class ClientSocketConnectionHelper
    {
        /// <summary>
        /// </summary>
        public ClientSocketConnectionHelper()
        {
        }

        /// <summary>
        /// </summary>
        /// <param name="socket">
        /// </param>
        /// <param name="remoteEndPoint">
        /// </param>
        public ClientSocketConnectionHelper(Socket socket, EndPoint remoteEndPoint)
        {
            ClientSocket = socket;
            RemoteEndPoint = remoteEndPoint;
        }

        /// <summary>
        /// </summary>
        public event EventHandler<SocketAsyncEventArgs> Connected;

        /// <summary>
        /// </summary>
        public EndPoint RemoteEndPoint { get; set; }

        /// <summary>
        /// </summary>
        public Socket ClientSocket { get; set; }

        /// <summary>
        /// </summary>
        public void StartConnection()
        {
            Debug.Assert(ClientSocket != null);
            Debug.Assert(RemoteEndPoint != null);
            Debug.Assert(Connected != null);

            SocketAsyncEventArgs asyncEventArgs = new SocketAsyncEventArgs();
            asyncEventArgs.RemoteEndPoint = RemoteEndPoint;
            asyncEventArgs.Completed += ConnectedHandler;

            Console.WriteLine("Attempting to connect to server {0}...", asyncEventArgs.RemoteEndPoint);
            if( !ClientSocket.ConnectAsync(asyncEventArgs) )
            {
                ConnectedHandler(this, asyncEventArgs);
            }
        }

        private void ConnectedHandler(object sender, SocketAsyncEventArgs e)
        {
            if( Connected != null )
            {
                if( sender == this )
                {
                    Console.WriteLine("Connected to the server synchronously.");
                }
                else
                {
                    Console.WriteLine("Connected to the server asynchronously.");
                }

                Connected.EndInvoke(Connected.BeginInvoke(this, e, null, null));
            }
            else
            {
                Console.WriteLine("warning: Accepted connection closed because ConnectionAccepted event was not set.");
                CloseConnection(e);
            }
        }

        private void CloseConnection(SocketAsyncEventArgs socketAsyncEventArgs)
        {
        }
    }
}

