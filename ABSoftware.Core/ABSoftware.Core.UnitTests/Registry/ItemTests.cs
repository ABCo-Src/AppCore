using ABSoftware.Core.Registry.Items;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Text;

namespace ABSoftware.Core.UnitTests.Registry
{
    [TestClass]
    public class ItemTests
    {
        [TestMethod]
        public void RegString()
        {
            var item = new RegString();
            bool eventRan = false;
            item.ItemChanged += (s, e) =>
            {
                Assert.AreEqual("Test", item.Data);
                eventRan = true;
            };

            item.Data = "Test";
            Assert.AreEqual("Test", item.Data);
            Assert.IsTrue(eventRan);
        }

        [TestMethod]
        public void RegInteger()
        {
            var item = new RegInteger();
            bool eventRan = false;
            item.ItemChanged += (s, e) =>
            {
                Assert.AreEqual(82682L, item.Data);
                eventRan = true;
            };

            item.Data = 82682;
            Assert.AreEqual(82682, item.Data);
            Assert.IsTrue(eventRan);
        }

        [TestMethod]
        public void RegDecimal()
        {
            var item = new RegDecimal();
            bool eventRan = false;
            item.ItemChanged += (s, e) =>
            {
                Assert.AreEqual(82682.652f, item.Data);
                eventRan = true;
            };

            item.Data = 82682.652f;
            Assert.AreEqual(82682.652f, item.Data);
            Assert.IsTrue(eventRan);
        }

        [TestMethod]
        public void RegBooleanGroup()
        {
            var item = new RegBooleanGroup();
            bool eventRan = false;
            item.BooleanItemChanged += (s, e) =>
            {
                Assert.AreEqual(7, e.Pos);
                eventRan = true;
            };

            item.SetItem(7, true);
            Assert.IsTrue(eventRan);

            Assert.IsTrue(item.GetItem(7));

            item.SetItem(7, false);
            Assert.IsFalse(item.GetItem(7));
        }
    }
}
