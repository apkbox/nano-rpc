namespace NanoRpc
{
    using System;
    using System.IO.Pipes;
    using System.Threading;

    /// <summary>
    /// </summary>
    public class ClientNamedPipeConnector
    {
        private readonly string _pipeName;
        private readonly string _machineName;
        private readonly ManualResetEvent _quitEvent = new ManualResetEvent(false);
        private Thread _connectionThread;

        public ClientNamedPipeConnector(string pipeName)
        {
            _pipeName = pipeName;
            _machineName = ".";
        }

        public ClientNamedPipeConnector(string pipeName, string machineName)
        {
            CodeContracts.NotNull(pipeName, "pipeName");
            CodeContracts.NotNullOrEmpty(pipeName, "pipeName");

            _pipeName = pipeName;

            if( string.IsNullOrEmpty(machineName) )
            {
                _machineName = ".";
            }
            else
            {
                _machineName = machineName;
            }
        }

        ~ClientNamedPipeConnector()
        {
            StopConnecting();
        }

        /// <summary>
        /// </summary>
        public event EventHandler<ClientPipeConnectedEventArgs> Connected;

        /// <summary>
        /// Starts connecting.
        /// </summary>
        public void StartConnecting()
        {
            lock( this )
            {
                _quitEvent.Reset();

                if( _connectionThread != null )
                {
                    return;
                }

                _connectionThread = new Thread(AttemptToConnectThreadProc);
                _connectionThread.Name = GetType().AssemblyQualifiedName + "Thread";
                _connectionThread.Start();
            }
        }

        /// <summary>
        /// Stops connecting. The method waits until connection thread terminates.
        /// </summary>
        public void StopConnecting()
        {
            Thread thread;
            lock( this )
            {
                _quitEvent.Set();
                thread = _connectionThread;
            }

            if( thread != null && Thread.CurrentThread != thread )
            {
                thread.Join();
            }
        }

        private void AttemptToConnectThreadProc()
        {
            NamedPipeClientStream stream = null;

            while( true )
            {
                if( stream == null )
                {
                    stream = new NamedPipeClientStream(
                        _machineName,
                        _pipeName,
                        PipeDirection.InOut,
                        PipeOptions.Asynchronous);
                }

                if( _quitEvent.WaitOne(0) )
                {
                    break;
                }

                try
                {
                    Console.WriteLine("Waiting for server to connect...");
                    stream.Connect(500);

                    if( Connected != null )
                    {
                        Console.WriteLine("Client connector opened the pipe.");
                        _quitEvent.Set(); // Stop the thread unless requested otherwise in the handler
                        Connected(this, new ClientPipeConnectedEventArgs(stream));
                        stream = null;

                        // Avoid creating new stream unless we sure the client called StartConnecting in the callback.
                        if( _quitEvent.WaitOne(0) )
                        {
                            break;
                        }
                    }
                    else
                    {
                        // TODO: Probably the correct way is to throw an exception in StartConnection.
                        Console.WriteLine("warning: The pipe closed because Connected event was not set.");
                        break;
                    }
                }
                catch( TimeoutException )
                {
                }
            }

            if( stream != null )
            {
                stream.Close();
            }

            lock( this )
            {
                _connectionThread = null;
            }
        }
    }
}

