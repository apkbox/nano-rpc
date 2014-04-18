namespace NanoRpc.Net.Test
{
    using System;

    using Moq;
    using NUnit.Framework;

    /// <summary>
    /// The rpc controller test.
    /// </summary>
    [TestFixture]
    public class RpcClientControllerTest
    {
#if !DEBUG
        [Test]
        public void NullClientTest()
        {
            var controller = new RpcClientController();
            var message = new RpcMessage.Builder().WithResult(RpcStatus.RpcSucceeded).Build();

            Assert.Throws(
                Is.TypeOf<InvalidOperationException>(),
                () => controller.Receive(message));
        }
#endif

        [Test]
        [Description("Tests for an incoming event.")]
        public void ReceiveEventTest()
        {
            var controller = new RpcClientController();
            var client = new Mock<IRpcClient>();
            var message = MessageExtensions.Message("ITest", "Test").Build();

            controller.Recipient = client.Object;

            controller.Receive(message);

            client.Verify(o => o.Receive(It.Is<RpcMessage>(e => e == message)), Times.Exactly(1));
        }

        [Test]
        [Description("Tests for invalid incoming event.")]
        public void ReceiveInvalidEventTest()
        {
            var controller = new RpcClientController();
            var client = new Mock<IRpcClient>();
            var message = MessageExtensions.Message("ITest", "Test", true).Build();

            controller.Recipient = client.Object;

            Assert.Throws(
                Is.TypeOf<RpcException>().And.Property("Status").EqualTo(RpcStatus.RpcProtocolError),
                () => controller.Receive(message));

            client.Verify(o => o.Receive(It.Is<RpcMessage>(e => e == message)), Times.Never());
        }

        [Test]
        [Description("Tests for incoming result.")]
        public void ReceiveSuccessfulResultTest()
        {
            var controller = new RpcClientController();
            var client = new Mock<IRpcClient>();
            var message = new RpcMessage.Builder().WithResult(RpcStatus.RpcSucceeded).Build();

            controller.Recipient = client.Object;

            controller.Receive(message);

            client.Verify(o => o.Receive(It.Is<RpcMessage>(e => e == message)), Times.Exactly(1));
        }

        [Test]
        [Description("Tests for a failed result.")]
        public void ReceiveFailedResultTest()
        {
            var controller = new RpcClientController();
            var client = new Mock<IRpcClient>();
            var message = new RpcMessage.Builder().WithResult(RpcStatus.RpcUnknownInterface).Build();

            controller.Recipient = client.Object;

            controller.Receive(message);

            client.Verify(o => o.Receive(It.Is<RpcMessage>(e => e == message)), Times.Exactly(1));
        }

        [Test]
        [Description("Tests for a channel failure.")]
        public void ReceiveChannelFailureTest()
        {
            var controller = new RpcClientController();
            var client = new Mock<IRpcClient>();
            var message = MessageExtensions.Message("ITest", "Test").WithResult(RpcStatus.RpcChannelFailure).Build();

            controller.Recipient = client.Object;

            controller.Receive(message);

            client.Verify(o => o.Receive(It.Is<RpcMessage>(e => e == message)), Times.Exactly(1));
        }

        [Test]
        [Description("Tests invalid message handling (call and successful result cannot coexist).")]
        public void ReceiveInvalidMessageTest()
        {
            var controller = new RpcClientController();
            var client = new Mock<IRpcClient>();
            var message = MessageExtensions.Message("ITest", "Test").WithResult(RpcStatus.RpcSucceeded).Build();

            controller.Recipient = client.Object;

            Assert.Throws(
                Is.TypeOf<RpcException>().And.Property("Status").EqualTo(RpcStatus.RpcProtocolError),
                () => controller.Receive(message));

            client.Verify(o => o.Receive(It.Is<RpcMessage>(e => e == message)), Times.Never());
        }
    }
}
