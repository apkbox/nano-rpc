namespace NanoRpc.Net.Test
{
    using System;
    using NUnit.Framework;

    /// <summary>
    /// The rpc controller test.
    /// </summary>
    [TestFixture]
    public class RpcControllerTest
    {
        private class RpcControllerImpl : RpcController
        {
            public override void Receive(RpcMessage rpcMessage)
            {
                throw new NotImplementedException();
            }
        }

        /// <summary>
        /// The channel mock.
        /// </summary>
        private class ChannelMock : RpcChannel
        {
            /// <summary>
            /// Gets StartCalled.
            /// </summary>
            public int StartCalled { get; private set; }

            /// <summary>
            /// Gets SendCalled.
            /// </summary>
            public int SendCalled { get; private set; }

            /// <summary>
            /// Gets SentMessage.
            /// </summary>
            public RpcMessage SentMessage { get; private set; }

            /// <summary>
            /// Initializes a new instance of the <see cref="ChannelMock"/> class.
            /// </summary>
            /// <param name="rpcController">
            /// The rpc controller.
            /// </param>
            public ChannelMock(RpcController rpcController) : base(rpcController)
            {
            }

            /// <summary>
            /// The start.
            /// </summary>
            public override void Start()
            {
                StartCalled++;
            }

            /// <summary>
            /// The send.
            /// </summary>
            /// <param name="rpcMessage">
            /// The rpc message.
            /// </param>
            internal override void Send(RpcMessage rpcMessage)
            {
                SendCalled++;
                SentMessage = rpcMessage;
            }
        }

#if !DEBUG
        [Test]
        public void NullChannelTest()
        {
            var controller = new RpcControllerImpl();
            var message = new RpcMessage();

            Assert.Throws(
                Is.TypeOf<InvalidOperationException>(),
                () => controller.Send(message));
        }
#endif

        [Test]
        public void SendTest()
        {
            var controller = new RpcControllerImpl();
            var channel = new ChannelMock(controller);
            var message = new RpcMessage();

            controller.Channel = channel;
            controller.Send(message);

            Assert.That(channel.SendCalled, Is.EqualTo(1));
            Assert.That(channel.SentMessage, Is.EqualTo(message));
        }
    }
}
