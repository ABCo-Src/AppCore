using ABSoftware.Core.Registry;
using ABSoftware.Core.Registry.Items;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Collections.Generic;
using System.Linq;

namespace ABSoftware.Core.UnitTests.Registry
{
    [TestClass]
    public class GroupTests
    {
        [TestMethod]
        public void Exists()
        {
            var group = SetupGroup();

            Assert.IsTrue(group.Exists("abc/def/ghi"));
            Assert.IsTrue(group.Exists("abc/def/"));
            Assert.IsFalse(group.Exists("abc/def"));
            Assert.IsFalse(group.Exists("abc/def/ghf"));
            Assert.IsFalse(group.Exists("abc/del/ghi"));
        }

        [TestMethod]
        public void GetItem()
        {
            var group = SetupGroup();

            Assert.IsTrue(group.TryGetItem("abc/def/ghi", out IRegItem item));
            Assert.AreEqual(1234, ((RegInteger)item).Data);

            Assert.IsTrue(group.TryGetItem("abc/def/", out IRegItem groupRes));
            Assert.AreEqual(group.SubGroups["abc"].SubGroups["def"], groupRes);

            Assert.IsFalse(group.TryGetItem("abc/def", out _));
        }

        [TestMethod]
        public void CreateGroup()
        {
            var group = SetupGroup();

            var newGroup = group.CreateGroup("abc/def/xxx/");
            Assert.AreEqual(group.SubGroups["abc"].SubGroups["def"].SubGroups["xxx"], newGroup);

            var res = group.TryCreateGroup("abc/de/xxx/", out _);
            Assert.AreEqual(AddItemResult.InvalidPath, res);

            res = group.TryCreateGroup("abc/def/xxx/", out _);
            Assert.AreEqual(AddItemResult.ItemAlreadyExists, res);
        }

        [TestMethod]
        public void AddItem()
        {
            var group = SetupGroup();
            var newDec = new RegDecimal() { Data = 9.35f };

            group.AddItem("abc/def/xxx", newDec);
            Assert.AreEqual(group.SubGroups["abc"].SubGroups["def"].SubItems["xxx"], newDec);

            var res = group.TryAddItem("abc/de/xxx", new RegDecimal());
            Assert.AreEqual(AddItemResult.InvalidPath, res);

            res = group.TryAddItem("abc/def/xxx", new RegDecimal());
            Assert.AreEqual(AddItemResult.ItemAlreadyExists, res);
        }

        [TestMethod]
        public void RemoveItem()
        {
            var group = SetupGroup();

            group.RemoveItem("abc/def/ghi");
            Assert.IsFalse(group.SubGroups["abc"].SubGroups["def"].SubItems.ContainsKey("ghi"));

            Assert.IsTrue(group.TryRemoveItem("abc/def/"));
            Assert.IsFalse(group.SubGroups["abc"].SubGroups.ContainsKey("def"));
        }

        public RegGroup SetupGroup()
        {
            return new RegGroup()
            {
                SubGroups = new Dictionary<string, RegGroup>()
                {
                    {
                        "abc",
                        new RegGroup()
                        {
                            SubGroups = new Dictionary<string, RegGroup>()
                            {
                                {
                                    "def",
                                    new RegGroup() {
                                        SubItems = new Dictionary<string, IRegItem>()
                                        {
                                            { "ghi", new RegInteger() { Data = 1234 } }
                                        }
                                    }

                                }
                            }

                        }
                    }
                }
            };
        }
    }
}
