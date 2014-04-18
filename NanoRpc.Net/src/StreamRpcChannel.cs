// <summary>
// Implements stream based channel.
// </summary>

//#define DEBUG_CONSOLE

namespace NanoRpc
{
    using System;
    using System.Diagnostics;
    using System.IO;
    using System.Threading;

    /// <summary>
    /// An RpcChannel that communicates over a Stream. Does not work with any Stream, it must be a bidrectional channel
    /// like NetworkStream.
    /// </summary>
    public class StreamRpcChannel : RpcChannel
    {
        private readonly Stream _stream;
        private readonly Thread _startIoThread;
        private readonly ManualResetEvent _disconnectEvent = new ManualResetEvent( false );

        /// <summary>
        /// </summary>
        /// <param name="controller">
        /// </param>
        /// <param name="stream">
        /// </param>
        public StreamRpcChannel(RpcController controller, Stream stream)
            : base(controller)
        {
            _stream = stream;
            _startIoThread = new Thread(StartIoThreadProc);
            IsConnected = true;
        }

        /// <summary>
        /// </summary>
        public event EventHandler Disconnected;

        /// <summary>
        /// </summary>
        public bool IsConnected { get; set; }

        /// <summary>
        /// </summary>
        public Stream Stream
        {
            get { return _stream; }
        }

        /// <summary>
        /// </summary>
        public override void Start()
        {
            _startIoThread.Start();
        }

        /// <summary>
        /// Closes all the streams and requests that the channel shutdown, then joins the channel threads until they
        /// have terminated.
        /// </summary>
        public void CloseAndJoin()
        {
            _disconnectEvent.Set();
            // readStream must be closed, it's the only way we can terminate the readThread cleanly. Even async reads
            // don't support cancellation
            // socket.Disconnect( true );
            // socket.Close( 2000 );
        }

        /// <summary>
        /// `
        /// </summary>
        /// <param name="message">
        /// </param>
        internal override void Send(RpcMessage message)
        {
            if( !IsConnected )
            {
                throw new RpcException(RpcStatus.RpcChannelFailure);
            }

            var memStream = new MemoryStream();

// TODO: For sake of compatibility use protobuf IO to serialize the message size.
            memStream.Write(BitConverter.GetBytes(message.SerializedSize), 0, sizeof( int ));
            message.WriteTo(memStream);
            _stream.Write(memStream.GetBuffer(), 0, (int)memStream.Position);

#if DEBUG_CONSOLE
            Console.WriteLine( "Sending message {0} of {1} bytes", message.Id, args.Buffer.Length );
#endif
        }

        private IAsyncResult StartWaitingForHeader( ReadOperationState readOperationState )
        {
            readOperationState.IsWaitingForHeader = true;
            readOperationState.EnsureBufferSize(sizeof(int));

#if DEBUG_CONSOLE
            Console.WriteLine("Waiting for message prolog");
#endif
            try
            {
                IAsyncResult asyncResult = _stream.BeginRead(readOperationState.Buffer, 0, sizeof(int),
                                                             HandleReceivedData, readOperationState);
#if DEBUG_CONSOLE
                Console.WriteLine("CompletedSynchronously: {0}, IsCompleted: {1}", asyncResult.CompletedSynchronously,
                                  asyncResult.IsCompleted);
#endif
                return asyncResult;
            }
            catch(InvalidOperationException)
            {
#if DEBUG_CONSOLE
                Console.WriteLine("Start: BeginRead thrown an exception");
#endif
                SendDisconnectEventAsynchronously();
            }

            return null;
        }

        private void HandleReceivedData(IAsyncResult asyncResult)
        {
            var readOperationState = asyncResult.AsyncState as ReadOperationState;
            // TODO: Handle OperationCancelled exception while attempting to call EndRead
            int bytesRead = _stream.EndRead(asyncResult);
            if( bytesRead != 0 )
            {
                if( readOperationState != null )
                {
                    if( readOperationState.IsWaitingForHeader )
                    {
                        readOperationState.IsWaitingForHeader = false;

// TODO: For sake of compatibility use protobuf IO to serialize the message size.
                        int expectedPacketSize = BitConverter.ToInt32(readOperationState.Buffer, 0);
                        readOperationState.EnsureBufferSize(expectedPacketSize);
                        _stream.BeginRead(
                                readOperationState.Buffer,
                                0,                              // Offset
                                expectedPacketSize,
                                HandleReceivedData,
                                readOperationState);

#if DEBUG_CONSOLE
                        Console.WriteLine("Prologue received, expecting message of {0} bytes", expectedPacketSize);
#endif
                    }
                    else
                    {
                        var memStream = new MemoryStream(readOperationState.Buffer, 0, bytesRead);

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
                        StartWaitingForHeader(readOperationState);

                        Receive(message);
#if DEBUG_CONSOLE
                        Console.WriteLine("Waiting for message prolog");
#endif
                    }

                    // TODO: If source is really fast, the we could dig pretty deep into the stack. So, avoid recursion.
#if DEBUG_CONSOLE
                    Console.WriteLine("Waiting for incoming messages");
#endif
                }
                else
                {
                    Debug.Assert(false, "ReadOperationState is null");
                }
            }
            else
            {
#if DEBUG_CONSOLE
                Console.WriteLine("Connection failure - cancelling pending calls. Reason: {0}", "None");
#endif
                IsConnected = false;
                RpcMessage failureMessage =
                    new RpcMessage.Builder().SetResult(
                        new RpcResult.Builder().SetStatus(RpcStatus.RpcChannelFailure).SetErrorMessage(
                            "RPC channel failure").Build()).Build();
                Receive(failureMessage);
                SendDisconnectEventAsynchronously();
            }
        }

        private void SendDisconnectEventAsynchronously()
        {
            _disconnectEvent.Set();
            if( Disconnected != null )
                Disconnected.EndInvoke(Disconnected.BeginInvoke(this, EventArgs.Empty, null, null));
        }

        private void StartIoThreadProc()
        {
            IAsyncResult asyncResult = StartWaitingForHeader( new ReadOperationState() );
            if(asyncResult != null)
                WaitHandle.WaitAny(new[] { asyncResult.AsyncWaitHandle, _disconnectEvent });
        }

        #region Nested type: ReadOperationState

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

        #endregion
    }
}

