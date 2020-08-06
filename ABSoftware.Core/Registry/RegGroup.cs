using ABSoftware.Core.Logging;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Runtime.CompilerServices;
using System.Text;

namespace ABSoftware.Core.Registry
{
    public enum AddItemResult
    {
        Success,
        InvalidPath,
        ItemAlreadyExists
    }

    internal struct ParseResult
    {
        public RegGroup ContainingGroup;
        public bool HasTrailingSlash;
        public string End;
    }

    public class RegGroup : IRegItem
    {
        public event EventHandler ItemChanged;

        internal Dictionary<string, RegGroup> SubGroups = new Dictionary<string, RegGroup>();
        internal Dictionary<string, IRegItem> SubItems = new Dictionary<string, IRegItem>();

        public unsafe bool Exists(string path) => TryGetItem(path, out _);

        public unsafe bool TryGetItem(string path, out IRegItem item)
        {
            char* buffer = stackalloc char[path.Length];
            return TryGetItemBuffer(path, 0, buffer, out item);
        }

        internal unsafe bool TryGetItemBuffer(string path, int pos, char* buffer, out IRegItem item)
        {
            item = null;

            if (!GetGroupAndEnd(path, pos, buffer, out ParseResult result))
                return false;

            if (result.HasTrailingSlash)
            {
                var found = result.ContainingGroup.ShallowGetGroup(result.End, out RegGroup group);
                item = group;
                return found;
            }
            else return result.ContainingGroup.ShallowGetItem(result.End, out item);
        }

        public unsafe AddItemResult TryAddItem(string path, IRegItem item)
        {
            char* buffer = stackalloc char[path.Length];
            return TryAddItemBuffer(path, 0, buffer, item);
        }

        internal unsafe AddItemResult TryAddItemBuffer(string path, int pos, char* buffer, IRegItem item)
        {
            if (!GetGroupAndEnd(path, pos, buffer, out ParseResult result)) return AddItemResult.InvalidPath;

            if (result.HasTrailingSlash)
            {
                if (result.ContainingGroup.SubGroups.ContainsKey(result.End)) return AddItemResult.ItemAlreadyExists;
                result.ContainingGroup.SubGroups.Add(result.End, (RegGroup)item);
            }
            else
            {
                if (result.ContainingGroup.SubItems.ContainsKey(result.End)) return AddItemResult.ItemAlreadyExists;
                result.ContainingGroup.SubItems.Add(result.End, item);
            }

            ItemChanged?.Invoke(this, new EventArgs());
            return AddItemResult.Success;
        }

        public unsafe bool TryRemoveItem(string path)
        {
            char* buffer = stackalloc char[path.Length];
            return TryRemoveItemBuffer(path, 0, buffer);
        }

        internal unsafe bool TryRemoveItemBuffer(string path, int pos, char* buffer)
        {
            if (!GetGroupAndEnd(path, pos, buffer, out ParseResult result)) return false;

            ItemChanged?.Invoke(this, new EventArgs());
            if (result.HasTrailingSlash) return result.ContainingGroup.SubGroups.Remove(result.End);
            else return result.ContainingGroup.SubItems.Remove(result.End);
        }

        public unsafe AddItemResult TryCreateGroup(string path, out RegGroup newGroup)
        {
            newGroup = new RegGroup();
            return TryAddItem(path, newGroup);
        }

        // For the text "abc/def/ghi", this would get the group for "def", and the ending "ghi".
        internal unsafe bool GetGroupAndEnd(string path, int pos, char* buffer, out ParseResult result)
        {
            char* bufferCurrent = buffer;
            for (; pos < path.Length; pos++)
            {
                if (path[pos] == '/')
                {
                    // Detect trailing slashes.
                    if (++pos == path.Length)
                    {
                        result = new ParseResult
                        {
                            HasTrailingSlash = true
                        };
                        goto EndOfString;
                    }

                    return NextPart(path, pos, buffer, (int)(bufferCurrent - buffer), out result);
                }
                else *bufferCurrent++ = path[pos];
            }

            result = new ParseResult();

        EndOfString:
            result.ContainingGroup = this;
            result.End = new string(buffer, 0, (int)(bufferCurrent - buffer));
            return true;
        }

        unsafe bool NextPart(string path, int pos, char* buffer, int lengthWritten, out ParseResult result)
        {
            var lastPart = new string(buffer, 0, lengthWritten);

            if (ShallowGetGroup(lastPart, out RegGroup thisGroup))
                return thisGroup.GetGroupAndEnd(path, pos, buffer, out result);
            else
            {
                result = new ParseResult();
                return false;
            }
        }

        #region Error Throwing Versions

        public unsafe IRegItem GetItem(string path)
        {
            if (!TryGetItem(path, out IRegItem item))
                ABSLog.ThrowError($"ABSoftware Core: Failed to get the item at path: '{path}'");

            return item;
        }

        public unsafe void AddItem(string path, IRegItem item)
        {
            if (TryAddItem(path, item) != AddItemResult.Success)
                ABSLog.ThrowError($"ABSoftware Core: Failed to add item at path: '{path}'");
        }

        public unsafe void RemoveItem(string path)
        {
            if (!TryRemoveItem(path))
                ABSLog.ThrowError($"ABSoftware Core: Failed to remove item at path: '{path}'");
        }

        public unsafe RegGroup CreateGroup(string path)
        {
            var newGroup = new RegGroup();
            AddItem(path, newGroup);
            return newGroup;
        }

        #endregion

        public bool ShallowGetGroup(string subGroup, out RegGroup res) => SubGroups.TryGetValue(subGroup, out res);
        public bool ShallowGetItem(string subItem, out IRegItem res) => SubItems.TryGetValue(subItem, out res);
        public ReadOnlyDictionary<string, RegGroup> GetSubGroups() => new ReadOnlyDictionary<string, RegGroup>(SubGroups);
        public ReadOnlyDictionary<string, IRegItem> GetSubItems() => new ReadOnlyDictionary<string, IRegItem>(SubItems);
    }
}
