namespace NanoRpc.Net.Test
{
    using Moq;
    using NUnit.Framework;

    public interface ITestInterface
    {
        bool BoolProperty { get; set; }
        int IntProperty { get; set; }
        long LongProperty { get; set; }
        double DoubleProperty { get; set; }
    }

    /// <summary>
    /// The rpc proxy generator test.
    /// </summary>
    [TestFixture]
    public class RpcProxyBuilderTest
    {
        [Test]
        [Combinatorial]
        public void BuildClientProxyPropBoolGetTest( [Values(true, false)] bool value )
        {
            var mockClient = new Mock<IRpcClient>(MockBehavior.Strict);
            mockClient.Setup(
                client =>
                client.SendWithResult(
                    MessageExtensions.Message("NanoRpc.Net.Test.ITestInterface", "get_BoolProperty")
                        .Build()))
                .Returns(
                    MessageExtensions.Result(
                        RpcStatus.RpcSucceeded,
                        MessageExtensions.Value(value))
                        .Build())
                .Verifiable();

            object proxy = RpcProxyBuilder.BuildSingletonObjectProxy(typeof( ITestInterface ), mockClient.Object);
            Assert.That(proxy, Is.Not.Null);
            Assert.That(proxy, Is.InstanceOf<ITestInterface>());
            Assert.That(((ITestInterface) proxy).BoolProperty, Is.EqualTo(value));
            mockClient.Verify(client => client.Send(It.IsAny<RpcMessage>()), Times.Never());
            mockClient.Verify(client => client.SendWithResult(It.IsAny<RpcMessage>()), Times.Exactly(1));
            mockClient.Verify();
        }

        [Test]
        [Combinatorial]
        public void BuildClientProxyPropBoolSetTest( [Values(true, false)] bool value )
        {
            var mockClient = new Mock<IRpcClient>();
            mockClient.Setup(client => client.Send(It.IsAny<RpcMessage>()));
            mockClient.Setup(
                client =>
                client.SendWithResult(
                    MessageExtensions.Message("NanoRpc.Net.Test.ITestInterface", "set_BoolProperty")
                        .WithParameter(value)
                        .Build()))
                .Returns(
                    MessageExtensions.Result(RpcStatus.RpcSucceeded)
                        .Build())
                .Verifiable();

            object proxy = RpcProxyBuilder.BuildSingletonObjectProxy(typeof( ITestInterface ), mockClient.Object);
            Assert.That(proxy, Is.Not.Null);
            Assert.That(proxy, Is.InstanceOf<ITestInterface>());
            ((ITestInterface) proxy).BoolProperty = value;
            mockClient.Verify(client => client.Send(It.IsAny<RpcMessage>()), Times.Never());
            mockClient.Verify(client => client.SendWithResult(It.IsAny<RpcMessage>()), Times.Exactly(1));
            mockClient.Verify();
        }

        [Test]
        [Combinatorial]
        public void BuildClientProxyPropIntGetTest([Values(int.MinValue, -3, -1, 0, 1, 3, int.MaxValue)] int value)
        {
            var mockClient = new Mock<IRpcClient>(MockBehavior.Strict);
            mockClient.Setup(
                client =>
                client.SendWithResult(
                    MessageExtensions.Message("NanoRpc.Net.Test.ITestInterface", "get_IntProperty")
                        .Build()))
                .Returns(
                    MessageExtensions.Result(
                        RpcStatus.RpcSucceeded,
                        MessageExtensions.Value(value))
                        .Build())
                .Verifiable();

            object proxy = RpcProxyBuilder.BuildSingletonObjectProxy(typeof(ITestInterface), mockClient.Object);
            Assert.That(proxy, Is.Not.Null);
            Assert.That(proxy, Is.InstanceOf<ITestInterface>());
            Assert.That(((ITestInterface)proxy).IntProperty, Is.EqualTo(value));
            mockClient.Verify(client => client.Send(It.IsAny<RpcMessage>()), Times.Never());
            mockClient.Verify(client => client.SendWithResult(It.IsAny<RpcMessage>()), Times.Exactly(1));
            mockClient.Verify();
        }

        [Test]
        [Combinatorial]
        public void BuildClientProxyPropIntSetTest([Values(int.MinValue, -3, -1, 0, 1, 3, int.MaxValue)] int value)
        {
            var mockClient = new Mock<IRpcClient>();
            mockClient.Setup(client => client.Send(It.IsAny<RpcMessage>()));
            mockClient.Setup(
                client =>
                client.SendWithResult(
                    MessageExtensions.Message("NanoRpc.Net.Test.ITestInterface", "set_IntProperty")
                        .WithParameter(value)
                        .Build()))
                .Returns(
                    MessageExtensions.Result(RpcStatus.RpcSucceeded)
                        .Build())
                .Verifiable();

            object proxy = RpcProxyBuilder.BuildSingletonObjectProxy(typeof( ITestInterface ), mockClient.Object);
            Assert.That(proxy, Is.Not.Null);
            Assert.That(proxy, Is.InstanceOf<ITestInterface>());
            ((ITestInterface) proxy).IntProperty = value;
            mockClient.Verify(client => client.Send(It.IsAny<RpcMessage>()), Times.Never());
            mockClient.Verify(client => client.SendWithResult(It.IsAny<RpcMessage>()), Times.Exactly(1));
            mockClient.Verify();
        }

        [Test]
        [Combinatorial]
        public void BuildClientProxyPropLongGetTest([Values(long.MinValue, -3, -1, 0, 1, 3, long.MaxValue)] long value)
        {
            var mockClient = new Mock<IRpcClient>(MockBehavior.Strict);
            mockClient.Setup(
                client =>
                client.SendWithResult(
                    MessageExtensions.Message("NanoRpc.Net.Test.ITestInterface", "get_LongProperty")
                        .Build()))
                .Returns(
                    MessageExtensions.Result(
                        RpcStatus.RpcSucceeded,
                        MessageExtensions.Value(value))
                        .Build())
                .Verifiable();

            object proxy = RpcProxyBuilder.BuildSingletonObjectProxy(typeof(ITestInterface), mockClient.Object);
            Assert.That(proxy, Is.Not.Null);
            Assert.That(proxy, Is.InstanceOf<ITestInterface>());
            Assert.That(((ITestInterface)proxy).LongProperty, Is.EqualTo(value));
            mockClient.Verify(client => client.Send(It.IsAny<RpcMessage>()), Times.Never());
            mockClient.Verify(client => client.SendWithResult(It.IsAny<RpcMessage>()), Times.Exactly(1));
            mockClient.Verify();
        }

        [Test]
        [Combinatorial]
        public void BuildClientProxyPropLongSetTest([Values(long.MinValue, -3, -1, 0, 1, 3, long.MaxValue)] long value)
        {
            var mockClient = new Mock<IRpcClient>();
            mockClient.Setup(client => client.Send(It.IsAny<RpcMessage>()));
            mockClient.Setup(
                client =>
                client.SendWithResult(
                    MessageExtensions.Message("NanoRpc.Net.Test.ITestInterface", "set_LongProperty")
                        .WithParameter(value)
                        .Build()))
                .Returns(
                    MessageExtensions.Result(RpcStatus.RpcSucceeded)
                        .Build())
                .Verifiable();

            object proxy = RpcProxyBuilder.BuildSingletonObjectProxy(typeof(ITestInterface), mockClient.Object);
            Assert.That(proxy, Is.Not.Null);
            Assert.That(proxy, Is.InstanceOf<ITestInterface>());
            ((ITestInterface)proxy).LongProperty = value;
            mockClient.Verify(client => client.Send(It.IsAny<RpcMessage>()), Times.Never());
            mockClient.Verify(client => client.SendWithResult(It.IsAny<RpcMessage>()), Times.Exactly(1));
            mockClient.Verify();
        }

        [Test]
        [Combinatorial]
        public void BuildClientProxyPropDoubleGetTest([Values(double.MinValue, -3.3, -1.1, 0, 1.1, 3.3, double.MaxValue)] double value)
        {
            var mockClient = new Mock<IRpcClient>(MockBehavior.Strict);
            mockClient.Setup(
                client =>
                client.SendWithResult(
                    MessageExtensions.Message("NanoRpc.Net.Test.ITestInterface", "get_DoubleProperty")
                        .Build()))
                .Returns(
                    MessageExtensions.Result(
                        RpcStatus.RpcSucceeded,
                        MessageExtensions.Value(value))
                        .Build())
                .Verifiable();

            object proxy = RpcProxyBuilder.BuildSingletonObjectProxy(typeof(ITestInterface), mockClient.Object);
            Assert.That(proxy, Is.Not.Null);
            Assert.That(proxy, Is.InstanceOf<ITestInterface>());
            Assert.That(((ITestInterface)proxy).DoubleProperty, Is.EqualTo(value));
            mockClient.Verify(client => client.Send(It.IsAny<RpcMessage>()), Times.Never());
            mockClient.Verify(client => client.SendWithResult(It.IsAny<RpcMessage>()), Times.Exactly(1));
            mockClient.Verify();
        }

        [Test]
        [Combinatorial]
        public void BuildClientProxyPropDoubleSetTest([Values(double.MinValue, -3.3, -1.1, 0, 1.1, 3.3, double.MaxValue)] double value)
        {
            var mockClient = new Mock<IRpcClient>();
            mockClient.Setup(client => client.Send(It.IsAny<RpcMessage>()));
            mockClient.Setup(
                client =>
                client.SendWithResult(
                    MessageExtensions.Message("NanoRpc.Net.Test.ITestInterface", "set_DoubleProperty")
                        .WithParameter(value)
                        .Build()))
                .Returns(
                    MessageExtensions.Result(RpcStatus.RpcSucceeded)
                        .Build())
                .Verifiable();

            object proxy = RpcProxyBuilder.BuildSingletonObjectProxy(typeof(ITestInterface), mockClient.Object);
            Assert.That(proxy, Is.Not.Null);
            Assert.That(proxy, Is.InstanceOf<ITestInterface>());
            ((ITestInterface)proxy).DoubleProperty = value;
            mockClient.Verify(client => client.Send(It.IsAny<RpcMessage>()), Times.Never());
            mockClient.Verify(client => client.SendWithResult(It.IsAny<RpcMessage>()), Times.Exactly(1));
            mockClient.Verify();
        }
    }
}

