namespace NanoRpc.Net.Test
{
    using Moq;
    using NUnit.Framework;

    [TestFixture]
    public class RpcObjectManagerTest
    {
        public interface IObject
        {
            int ObjectMethod();
        }

        [Test]
        public void CanRegisterInstance()
        {
            var objectManager = new RpcObjectManager();
            var obj = new Mock<IObject>();

            var oid = objectManager.RegisterInstance(obj.Object, typeof(IObject));
            Assert.That(oid, Is.GreaterThan(0));
            Assert.That(objectManager.GetInstance(oid), Is.Not.Null);
            Assert.That(objectManager.GetInstance(oid), Is.Not.SameAs(obj.Object));
        }

        [Test]
        public void CanRegisterStubInstance()
        {
            var objectManager = new RpcObjectManager();
            var stub = new Mock<IRpcService>();

            var oid = objectManager.RegisterInstance(stub.Object, typeof(IRpcService));
            Assert.That(oid, Is.GreaterThan(0));
            Assert.That(objectManager.GetInstance(oid), Is.Not.Null);
            Assert.That(objectManager.GetInstance(oid), Is.SameAs(stub.Object));
        }

        [Test]
        public void TwoRegisteredInstancesAreDifferent()
        {
            var objectManager = new RpcObjectManager();
            var objectA = new Mock<IObject>();
            var objectB = new Mock<IObject>();

            var oidA = objectManager.RegisterInstance(objectA.Object, typeof(IObject));
            var oidB = objectManager.RegisterInstance(objectB.Object, typeof(IObject));
            Assert.That(oidA, Is.GreaterThan(0));
            Assert.That(oidB, Is.GreaterThan(0));
            Assert.That(oidA, Is.Not.EqualTo(oidB));
            Assert.That(objectManager.GetInstance(oidA), Is.Not.Null);
            Assert.That(objectManager.GetInstance(oidB), Is.Not.Null);
            Assert.That(objectManager.GetInstance(oidA), Is.Not.EqualTo(objectManager.GetInstance(oidB)));
        }

        [Test]
        public void AttemptToRetrieveNonexistingInstance()
        {
            var objectManager = new RpcObjectManager();
            Assert.That(objectManager.GetInstance(42), Is.Null);
        }

        [Test]
        public void ZeroObjectIdIsForbidden()
        {
            var objectManager = new RpcObjectManager();
            Assert.That(objectManager.GetInstance(0), Is.Null);
        }

        [Test]
        public void ObjectInstanceIsNotService()
        {
            var objectManager = new RpcObjectManager();
            var obj = new Mock<IObject>();

            objectManager.RegisterInstance(obj.Object, typeof(IObject));

            Assert.That(objectManager.GetService(typeof(IObject).FullName), Is.Null);
        }

        [Test]
        public void CanRegisterService()
        {
            var objectManager = new RpcObjectManager();
            var obj = new Mock<IObject>();

            objectManager.RegisterService(typeof(IObject), obj.Object);
            Assert.That(objectManager.GetService(typeof(IObject).FullName), Is.Not.Null);
            Assert.That(objectManager.GetService(typeof(IObject).FullName), Is.AssignableTo<IRpcService>());
        }

        [Test]
        public void CanRegisterServiceViaTemplate()
        {
            var objectManager = new RpcObjectManager();
            var obj = new Mock<IObject>();

            objectManager.RegisterService(obj.Object);
            Assert.That(objectManager.GetService(typeof(IObject).FullName), Is.Not.Null);
            Assert.That(objectManager.GetService(typeof(IObject).FullName), Is.AssignableTo<IRpcService>());
        }

        [Test]
        public void CanRegisterServiceWithCustomStub()
        {
            var objectManager = new RpcObjectManager();
            var obj = new Mock<IObject>();
            var stub = obj.As<IRpcService>();

            objectManager.RegisterService(typeof(IObject), stub.Object);
            Assert.That(objectManager.GetService(typeof(IObject).FullName), Is.Not.Null);
            Assert.That(objectManager.GetService(typeof(IObject).FullName), Is.AssignableTo<IRpcService>());
            Assert.That(objectManager.GetService(typeof(IObject).FullName), Is.SameAs(stub.Object));
        }

        [Test]
        public void NewServiceReplacesPreviouslyRegistered()
        {
            var objectManager = new RpcObjectManager();
            var callMethod = MessageExtensions.Message(typeof( IObject ).FullName, "ObjectMethod").Build().Call;

            // Register one service
            var serviceInstanceA = new Mock<IObject>();
            serviceInstanceA.Setup( o => o.ObjectMethod() ).Returns(42);
            objectManager.RegisterService(typeof(IObject), serviceInstanceA.Object);
            var serviceA = objectManager.GetService(typeof( IObject ).FullName);
            var resultA = serviceA.CallMethod(callMethod);

            // Register another service with the same interface
            var serviceInstanceB = new Mock<IObject>();
            serviceInstanceB.Setup( o => o.ObjectMethod() ).Returns(24);
            objectManager.RegisterService(typeof(IObject), serviceInstanceB.Object);
            var serviceB = objectManager.GetService(typeof( IObject ).FullName);
            var resultB = serviceB.CallMethod(callMethod);

            Assert.That(resultA.CallResult.Int32Value, Is.EqualTo(42));

            // Second registration overrides the first one.
            Assert.That(resultB.CallResult.Int32Value, Is.EqualTo(24));
        }

        [Test]
        public void DeleteInstanceRequest()
        {
            var objectManager = new RpcObjectManager();
            var objectInstanceMock = new Mock<IObject>();
            uint objectId = objectManager.RegisterInstance(objectInstanceMock.Object, typeof(IObject));

            var callMethod = MessageExtensions.Message(typeof( RpcObjectManager ).FullName, "Delete").WithParameter(objectId).Build();

            var result = objectManager.CallMethod(callMethod.Call);

            Assert.That(result.Status, Is.EqualTo(RpcStatus.RpcSucceeded));
            Assert.That(objectManager.GetInstance(objectId), Is.Null);
        }

        [Test]
        public void CallNonExistingMethod()
        {
            var objectManager = new RpcObjectManager();
            var objectInstanceMock = new Mock<IObject>();
            uint objectId = objectManager.RegisterInstance(objectInstanceMock.Object, typeof(IObject));

            var callMethod = MessageExtensions.Message(typeof( RpcObjectManager ).FullName, "NonExistingMethod").WithParameter(objectId).Build();

            var result = objectManager.CallMethod(callMethod.Call);

            Assert.That(result.Status, Is.EqualTo(RpcStatus.RpcUnknownMethod));
            Assert.That(objectManager.GetInstance(objectId), Is.Not.Null);
        }
    }
}

