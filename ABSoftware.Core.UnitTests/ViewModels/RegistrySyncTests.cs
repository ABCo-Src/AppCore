using ABSoftware.Core.Registry;
using ABSoftware.Core.Registry.Items;
using ABSoftware.Core.ViewModels;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Text;

namespace ABSoftware.Core.UnitTests.ViewModels
{
    [TestClass]
    public class RegistrySyncTests
    {
        public RegString StringItem;
        public RegInteger IntegerItem;
        public RegDecimal DecimalItem;
        public RegBooleanGroup BooleanGroupItem;

        [TestInitialize]
        public void SetupRegistry()
        {
            ABSRegistry.LoadedSegments.Clear();

            var newSegment = new RegSegment();

            StringItem = new RegString() { Data = "InRegistry" };
            IntegerItem = new RegInteger() { Data = 12 };
            DecimalItem = new RegDecimal() { Data = 12.3f };
            BooleanGroupItem = new RegBooleanGroup();

            BooleanGroupItem.SetItem(5, true);
            BooleanGroupItem.SetItem(6, false);

            ABSRegistry.LoadedSegments.Add("abc", newSegment);
            newSegment.AddItem("stringItm", StringItem);
            newSegment.AddItem("integerItm", IntegerItem);
            newSegment.AddItem("decimalItm", DecimalItem);
            newSegment.AddItem("booleanGroupItm", BooleanGroupItem);
        }

        [TestMethod]
        public void StringSync()
        {
            var syncVM = new RegistrySyncVM();

            Assert.AreEqual("InRegistry", syncVM.StringRegItem);
            syncVM.StringRegItem = "InProperty";

            Assert.AreEqual("InProperty", StringItem.Data);

            StringItem.Data = "InRegistry2";
            Assert.AreEqual("InRegistry2", syncVM.StringRegItem);
        }

        [TestMethod]
        public void IntegerSync()
        {
            var syncVM = new RegistrySyncVM();

            Assert.AreEqual(12, syncVM.IntegerRegItem);
            syncVM.IntegerRegItem = 34;

            Assert.AreEqual(34, IntegerItem.Data);

            IntegerItem.Data = 56;
            Assert.AreEqual(56, syncVM.IntegerRegItem);
        }

        [TestMethod]
        public void DecimalSync()
        {
            var syncVM = new RegistrySyncVM();

            Assert.AreEqual(12.3f, syncVM.DecimalRegItem);
            syncVM.DecimalRegItem = 45.6f;

            Assert.AreEqual(45.6f, DecimalItem.Data);

            DecimalItem.Data = 67.8f;
            Assert.AreEqual(67.8f, syncVM.DecimalRegItem);
        }

        [TestMethod]
        public void BooleanGroupSync()
        {
            var syncVM = new RegistrySyncVM();

            Assert.AreEqual(true, syncVM.BoolRegItem5);
            Assert.AreEqual(false, syncVM.BoolRegItem6);

            syncVM.BoolRegItem5 = false;
            syncVM.BoolRegItem6 = true;
            Assert.AreEqual(false, BooleanGroupItem.GetItem(5));
            Assert.AreEqual(true, BooleanGroupItem.GetItem(6));

            BooleanGroupItem.SetItem(5, true);
            BooleanGroupItem.SetItem(6, false);
            Assert.AreEqual(true, syncVM.BoolRegItem5);
            Assert.AreEqual(false, syncVM.BoolRegItem6);
        }
    }

    class RegistrySyncVM : BaseViewModel<RegistrySyncVM>
    {
        [RegistrySync("abc:stringItm")]
        public string StringRegItem { get; set; }

        [RegistrySync("abc:integerItm")]
        public int IntegerRegItem { get; set; }

        [RegistrySync("abc:decimalItm")]
        public float DecimalRegItem { get; set; }

        [RegistrySync("abc:booleanGroupItm", 5)]
        public bool BoolRegItem5 { get; set; }

        [RegistrySync("abc:booleanGroupItm", 6)]
        public bool BoolRegItem6 { get; set; }
    }
}
