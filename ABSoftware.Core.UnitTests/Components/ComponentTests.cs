using ABSoftware.Core.Components;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Text;

namespace ABSoftware.Core.UnitTests.Components
{
    [TestClass]
    public class ComponentTests
    {
        [TestMethod]
        public void CategoryOperations()
        {
            var category = ComponentManager.AddCategory("abc");
            Assert.IsTrue(ComponentManager.TryGetCategory("abc", out _));

            List<CategoryChangedEventArgs> args = new List<CategoryChangedEventArgs>();

            category.CategoryChanged += (s, e) =>
            {
                args.Add(e);
            };

            var l = category.AddList();
            var l2 = category.AddList();
            category.RemoveList(1);

            Assert.AreEqual(CategoryChangedType.Add, args[0].Type);
            Assert.AreEqual(l, args[0].ItemAdded);

            Assert.AreEqual(CategoryChangedType.Add, args[1].Type);
            Assert.AreEqual(l2, args[1].ItemAdded);

            Assert.AreEqual(CategoryChangedType.Remove, args[2].Type);
            Assert.AreEqual(1, args[2].ItemRemoved);

            ComponentManager.RemoveCategory("abc");
            Assert.IsFalse(ComponentManager.TryGetCategory("abc", out _));
        }

        [TestMethod]
        public void ListOperations()
        {
            var list = ComponentManager.AddCategory("x").AddList();
            List<ComponentListUpdatedEventArgs> args = new List<ComponentListUpdatedEventArgs>();

            list.Updated += (s, e) =>
            {
                args.Add(e);
            };

            var newComponent = new TestComponent();
            var newComponent2 = new TestComponent();
            list.RegisterComponent(newComponent);
            list.RegisterComponent(newComponent);
            list.ReregisterComponent(newComponent2, 1);
            list.UnregisterComponent(1);

            Assert.AreEqual(ComponentListUpdatedType.Register, args[0].Type);
            Assert.AreEqual(newComponent, args[0].Registered);

            Assert.AreEqual(ComponentListUpdatedType.Register, args[1].Type);
            Assert.AreEqual(newComponent, args[1].Registered);

            Assert.AreEqual(ComponentListUpdatedType.Reregister, args[2].Type);
            Assert.AreEqual(newComponent2, args[2].Registered);
            Assert.AreEqual(1, args[2].Index);

            Assert.AreEqual(ComponentListUpdatedType.Unregister, args[3].Type);
            Assert.AreEqual(1, args[3].Index);
        }
    }
}
