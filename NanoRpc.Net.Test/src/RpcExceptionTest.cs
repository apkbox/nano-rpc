namespace NanoRpc.Net.Test
{
    using NUnit.Framework;

    [TestFixture]
    public class RpcExceptionTest
    {
        [Test]
        [Combinatorial]
        public void CanInitialize([Values(RpcStatus.RpcUnknownInterface, RpcStatus.RpcUnknownMethod)] RpcStatus status)
        {
            var exception = new RpcException(status);
            Assert.That(exception.Status, Is.EqualTo(status));
        }

        [Test]
        [Combinatorial]
        public void CanInitializeWithMessage([Values("message1", "message2")] string message)
        {
            var exception = new RpcException(RpcStatus.RpcUnknownInterface, message);
            Assert.That(exception.Message, Is.EqualTo(message));
        }
    }
}

