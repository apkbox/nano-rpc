namespace NanoRpc
{
    using System;
    using System.Threading;

    /// <summary>
    /// </summary>
    internal class PendingCall : IAsyncResult
    {
        private static int _nextId;
        private readonly EventWaitHandle _asyncCompletionEvent = new ManualResetEvent(false);

        /// <summary>
        /// </summary>
        static PendingCall()
        {
            _nextId = new Random().Next();
        }

        /// <summary>
        /// </summary>
        public PendingCall()
        {
            Id = GetNextId();
        }

        /// <summary>
        /// </summary>
        public int Id { get; private set; }

        /// <summary>
        /// </summary>
        public RpcResult Result { get; private set; }

        #region IAsyncResult Members

        /// <summary>
        /// </summary>
        public bool IsCompleted
        {
            get { return Result != null; }
        }

        /// <summary>
        /// </summary>
        public WaitHandle AsyncWaitHandle
        {
            get { return _asyncCompletionEvent; }
        }

        /// <summary>
        /// This method always returns <c>null</c> reference.
        /// </summary>
        public object AsyncState
        {
            get { return null; }
        }

        /// <summary>
        /// </summary>
        public bool CompletedSynchronously
        {
            get { return false; }
        }

        #endregion

        /// <summary>
        /// </summary>
        /// <param name="rpcResult">
        /// </param>
        public void ReceiveResult(RpcResult rpcResult)
        {
            Result = rpcResult;
            _asyncCompletionEvent.Set();
        }

        private static int GetNextId()
        {
            return _nextId++;
        }
    }
}

