namespace NanoRpc.Net.Test
{
    using NUnit.Framework;

    namespace InnerNamespace1
    {
        public interface ITestInterface1
        {
            void MethodA();
        }

        public interface ITestInterface2
        {
            void MethodB();
        }
    }

    namespace InnerNamespace2
    {
        public interface ITestInterface1
        {
            void MethodA();
        }

        public interface ITestInterface2
        {
            void MethodB();
        }
    }

    /// <summary>
    /// The rpc proxy generator test.
    /// </summary>
    [TestFixture]
    public class RpcProxyGeneratorTest
    {
        [Test]
        [Description("Generate two proxies in the same namespace.")]
        public void SingleNamespaceMultipleInterfaces()
        {
            Assert.DoesNotThrow(() => RpcProxyGenerator.GetProxyType(typeof( InnerNamespace1.ITestInterface1 ), false));
            Assert.DoesNotThrow(() => RpcProxyGenerator.GetProxyType(typeof( InnerNamespace1.ITestInterface2 ), false));
        }

        [Test]
        [Description("Generate proxies in different namespaces.")]
        public void MultipleNamespaceSingleInterface()
        {
            Assert.DoesNotThrow(() => RpcProxyGenerator.GetProxyType(typeof( InnerNamespace1.ITestInterface1 ), false));
            Assert.DoesNotThrow(() => RpcProxyGenerator.GetProxyType(typeof( InnerNamespace2.ITestInterface1 ), false));
        }

        [Test]
        [Description("Generate multiple proxies in multiple namespaces.")]
        public void MultipleNamespaceMultipleInterfaces()
        {
            Assert.DoesNotThrow(() => RpcProxyGenerator.GetProxyType(typeof( InnerNamespace1.ITestInterface1 ), false));
            Assert.DoesNotThrow(() => RpcProxyGenerator.GetProxyType(typeof( InnerNamespace1.ITestInterface2 ), false));
            Assert.DoesNotThrow(() => RpcProxyGenerator.GetProxyType(typeof( InnerNamespace2.ITestInterface1 ), false));
            Assert.DoesNotThrow(() => RpcProxyGenerator.GetProxyType(typeof( InnerNamespace2.ITestInterface2 ), false));
        }

        [Test]
        [Description("Tests that generated proxy types are cached properly.")]
        public void TypeCaching()
        {
            var type111 = RpcProxyGenerator.GetProxyType(typeof(InnerNamespace1.ITestInterface1), false);
            var type121 = RpcProxyGenerator.GetProxyType(typeof(InnerNamespace1.ITestInterface2), false);
            var type211 = RpcProxyGenerator.GetProxyType(typeof(InnerNamespace2.ITestInterface1), false);
            var type212 = RpcProxyGenerator.GetProxyType(typeof(InnerNamespace2.ITestInterface1), false);
            var type112 = RpcProxyGenerator.GetProxyType(typeof(InnerNamespace1.ITestInterface1), false);
            var type113 = RpcProxyGenerator.GetProxyType(typeof(InnerNamespace1.ITestInterface1), false);

            // Assert returned types are correct.
            Assert.True(typeof( InnerNamespace1.ITestInterface1 ).IsAssignableFrom(type111));
            Assert.True(typeof( InnerNamespace1.ITestInterface2 ).IsAssignableFrom(type121));
            Assert.True(typeof( InnerNamespace2.ITestInterface1 ).IsAssignableFrom(type211));
            Assert.True(typeof( InnerNamespace2.ITestInterface1 ).IsAssignableFrom(type212));
            Assert.True(typeof( InnerNamespace1.ITestInterface1 ).IsAssignableFrom(type112));
            Assert.True(typeof( InnerNamespace1.ITestInterface1 ).IsAssignableFrom(type113));

            // And cached instances are the same.
            Assert.AreNotSame(type111, type121);
            Assert.AreNotSame(type111, type211);
            Assert.AreNotSame(type111, type212);
            Assert.AreSame(type111, type112);
            Assert.AreSame(type111, type113);

            Assert.AreNotSame(type121, type211);
            Assert.AreNotSame(type121, type212);
            Assert.AreNotSame(type121, type112);
            Assert.AreNotSame(type121, type113);

            Assert.AreSame(type211, type212);
            Assert.AreNotSame(type211, type112);
            Assert.AreNotSame(type211, type113);

            Assert.AreNotSame(type212, type112);
            Assert.AreNotSame(type212, type113);

            Assert.AreSame(type112, type113);
        }
    }
}

