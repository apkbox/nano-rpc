namespace NanoRpc
{
    using System;
    using System.IO.Pipes;

    public class ClientPipeConnectedEventArgs : EventArgs
    {
        public ClientPipeConnectedEventArgs( NamedPipeClientStream stream )
        {
            Stream = stream;
        }

        public NamedPipeClientStream Stream { get; private set; }
    }
}