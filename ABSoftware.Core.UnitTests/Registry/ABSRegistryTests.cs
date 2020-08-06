using ABSoftware.Core.Registry;
using ABSoftware.Core.Registry.Items;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Text;

namespace ABSoftware.Core.UnitTests.Registry
{
    [TestClass]
    public class ABSRegistryTests
    {
        [TestMethod]
        public unsafe void GetSegment_NotAlias_Valid()
        {
            var newSegment = new RegSegment();
            ABSRegistry.LoadedSegments.Add("abc", newSegment);
            char* ch = stackalloc char[4];
            Assert.IsTrue(ABSRegistry.GetSegment("abc:", ch, out int pos, out RegSegment segment));

            Assert.AreEqual(newSegment, segment);
            Assert.AreEqual(4, pos);
        }

        [TestMethod]
        public unsafe void GetSegment_Alias_Valid()
        {
            var newSegment = new RegSegment();
            ABSRegistry.SegmentAliases.Add("abc", newSegment);
            char* ch = stackalloc char[4];
            Assert.IsTrue(ABSRegistry.GetSegment("$abc:", ch, out int pos, out RegSegment segment));

            Assert.AreEqual(newSegment, segment);
            Assert.AreEqual(5, pos);
        }

        [TestMethod]
        public unsafe void GetSegment_Invalid()
        {
            char* ch = stackalloc char[4];

            Assert.IsFalse(ABSRegistry.GetSegment("$AG:", ch, out _, out _));
            Assert.IsFalse(ABSRegistry.GetSegment("XYZ:", ch, out _, out _));
            Assert.IsFalse(ABSRegistry.GetSegment("abc", ch, out _, out _));
        }
    }
}
