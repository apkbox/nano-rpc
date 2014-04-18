namespace NanoRpc.Net.Test
{
    using Moq;
    using NUnit.Framework;

    /// <summary>
    /// The rpc proxy helpers test.
    /// </summary>
    [TestFixture]
    public class RpcProxyHelpersTest
    {
        /// <summary>
        /// The method infos test.
        /// </summary>
        [Test]
        public void MethodInfosTest()
        {
            Assert.That(RpcProxyHelpers.BuildRpcMessageMethodInfo, Is.Not.Null);
            Assert.That(RpcProxyHelpers.CreateServiceRpcCallBuilderMethodInfo, Is.Not.Null);
            Assert.That(RpcProxyHelpers.CreateObjectRpcCallBuilderMethodInfo, Is.Not.Null);
            Assert.That(RpcProxyHelpers.BuildProxyMethodInfo, Is.Not.Null);
            Assert.That(RpcProxyHelpers.SendDeleteMessageMethodInfo, Is.Not.Null);
        }

        [Test]
        public void BuildRpcMessageTest()
        {
            var callBuilder = new RpcCall.Builder()
                           {
                               Service = "Service",
                               Method = "Method"
                           };
            var message = RpcProxyHelpers.BuildRpcMessage(callBuilder);
            Assert.That(message, Is.Not.Null);
            Assert.That(message.Call.Service, Is.EqualTo("Service"));
            Assert.That(message.Call.Method, Is.EqualTo("Method"));
        }

        [Test]
        public void CreateServiceRpcCallBuilderTest()
        {
            var callBuilder = RpcProxyHelpers.CreateServiceRpcCallBuilder("Service", "Method");
            Assert.That(callBuilder, Is.Not.Null);
            var call = callBuilder.Build();
            Assert.That(call.Service, Is.EqualTo("Service"));
            Assert.That(call.Method, Is.EqualTo("Method"));
            Assert.That(call.ObjectId, Is.EqualTo(0));
        }

        [Test]
        public void CreateObjectRpcCallBuilderTest()
        {
            var callBuilder = RpcProxyHelpers.CreateObjectRpcCallBuilder(12, "Method");
            Assert.That(callBuilder, Is.Not.Null);
            var call = callBuilder.Build();
            Assert.That(call.HasService, Is.False);
            Assert.That(call.Method, Is.EqualTo("Method"));
            Assert.That(call.ObjectId, Is.EqualTo(12));
        }

        [Test]
        public void CreateRpcParameterBuilderProtoTest()
        {
            var rpcMessage = new RpcMessage();
            var serializedRpcMessage = rpcMessage.ToByteString();

            var parameterBuilder = RpcProxyHelpers.CreateRpcParameterBuilder(rpcMessage);
            Assert.That(parameterBuilder, Is.Not.Null);
            
            var parameter = parameterBuilder.Build();
            Assert.That(parameter.ProtoValue.ToByteArray(), Is.EqualTo(serializedRpcMessage.ToByteArray()));
        }

        [Test]
        [Combinatorial]
        public void CreateRpcParameterBuilderBoolTest([Values(true, false)] bool value)
        {
            var parameterBuilder = RpcProxyHelpers.CreateRpcParameterBuilder(value);
            Assert.That(parameterBuilder, Is.Not.Null);
            var parameter = parameterBuilder.Build();
            Assert.That(parameter.BoolValue, Is.EqualTo(value));
        }

        [Test]
        [Combinatorial]
        public void CreateRpcParameterBuilderIntTest(
            [Values(int.MinValue, -3, -1, 0, 1, 3, int.MaxValue)] int value)
        {
            var parameterBuilder = RpcProxyHelpers.CreateRpcParameterBuilder(value);
            Assert.That(parameterBuilder, Is.Not.Null);
            var parameter = parameterBuilder.Build();
            Assert.That(parameter.Int32Value, Is.EqualTo(value));
        }

        [Test]
        [Combinatorial]
        public void CreateRpcParameterBuilderLongTest(
            [Values(long.MinValue, -3, -1, 0, 1, 3, long.MaxValue)] long value)
        {
            var parameterBuilder = RpcProxyHelpers.CreateRpcParameterBuilder(value);
            Assert.That(parameterBuilder, Is.Not.Null);
            var parameter = parameterBuilder.Build();
            Assert.That(parameter.Int64Value, Is.EqualTo(value));
        }

        [Test]
        [Combinatorial]
        public void CreateRpcParameterBuilderDoubleTest(
            [Values(double.MinValue, -3.3, -1.1, 0, 1.1, 3.3, double.MaxValue)] double value)
        {
            var parameterBuilder = RpcProxyHelpers.CreateRpcParameterBuilder(value);
            Assert.That(parameterBuilder, Is.Not.Null);
            var parameter = parameterBuilder.Build();
            Assert.That(parameter.DoubleValue, Is.EqualTo(value));
        }

        [Test]
        [Combinatorial]
        public void CreateRpcParameterBuilderStringTest(
            [Values("","String1", "String 2")] string value)
        {
            var parameterBuilder = RpcProxyHelpers.CreateRpcParameterBuilder(value);
            Assert.That(parameterBuilder, Is.Not.Null);
            var parameter = parameterBuilder.Build();
            Assert.That(parameter.StringValue, Is.EqualTo(value));
        }

        [Test]
        public void SendDeleteMessageSingletonObjectTest()
        {
            var mockClient = new Mock<IRpcClient>();
            RpcProxyHelpers.SendDeleteMessage(0, mockClient.Object);
            mockClient.Verify(client => client.SendWithResult(It.IsAny<RpcMessage>()), Times.Never());
            mockClient.Verify(client => client.Send(It.IsAny<RpcMessage>()), Times.Never());
        }

        [Test]
        public void SendDeleteMessageTransientObjectTest()
        {
            var mockClient = new Mock<IRpcClient>();
            RpcProxyHelpers.SendDeleteMessage(12, mockClient.Object);
            mockClient.Verify(client => client.SendWithResult(It.IsAny<RpcMessage>()), Times.Never());
            mockClient.Verify(
                client =>
                client.Send(
                    new RpcMessage.Builder()
                        .SetCall(
                            new RpcCall.Builder()
                                .SetService("NanoRpc.ObjectManagerService")
                                .SetMethod("Delete")
                                .AddParameters(
                                    new RpcParameter.Builder()
                                        .SetUint32Value(12)))
                        .Build()),
                Times.Once());
        }
    }
}

