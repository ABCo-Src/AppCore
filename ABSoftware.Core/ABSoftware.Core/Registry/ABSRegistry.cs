using ABSoftware.Core.Logging;
using System;
using System.Collections.Generic;
using System.Text;

namespace ABSoftware.Core.Registry
{
    public static class ABSRegistry
    {
        public static Dictionary<string, RegSegment> LoadedSegments = new Dictionary<string, RegSegment>();
        public static Dictionary<string, RegSegment> SegmentAliases = new Dictionary<string, RegSegment>();

        #region Path Access

        public static unsafe bool Exists(string path) => TryGetItem(path, out _);

        public static unsafe bool TryGetItem(string path, out IRegItem item)
        {
            item = null;
            char* buffer = stackalloc char[path.Length];

            if (!GetSegment(path, buffer, out int pos, out RegSegment segment)) return false;
            return segment.TryGetItemBuffer(path, pos, buffer, out item);
        }

        public static unsafe AddItemResult TryAddItem(string path, IRegItem item)
        {
            item = null;
            char* buffer = stackalloc char[path.Length];

            if (!GetSegment(path, buffer, out int pos, out RegSegment segment)) return AddItemResult.InvalidPath;
            return segment.TryAddItemBuffer(path, pos, buffer, item);
        }

        public static unsafe bool TryRemoveItem(string path)
        {
            char* buffer = stackalloc char[path.Length];

            if (!GetSegment(path, buffer, out int pos, out RegSegment segment)) return false;
            return segment.TryRemoveItemBuffer(path, pos, buffer);
        }

        public static unsafe AddItemResult TryCreateGroup(string path, out RegGroup newGroup)
        {
            newGroup = new RegGroup();
            char* buffer = stackalloc char[path.Length];

            if (!GetSegment(path, buffer, out int pos, out RegSegment segment)) return AddItemResult.InvalidPath;
            return segment.TryAddItemBuffer(path, pos, buffer, newGroup);
        }

        internal static unsafe bool GetSegment(string path, char* buffer, out int pos, out RegSegment segment)
        {
            pos = 0;
            segment = null;
            if (path.Length == 0) return false;

            bool isAlias = false;
            if (path[0] == '$')
            {
                pos++;
                isAlias = true;
            }

            // Get the section's name.
            char* currentBuffer = buffer;
            for (; pos < path.Length; pos++)
            {
                if (path[pos] == ':')
                {
                    pos++;
                    return GetSegmentByName(new string(buffer, 0, (int)(currentBuffer - buffer)), isAlias, out segment);
                }
                else *currentBuffer++ = path[pos];
            }

            // If we got to the end of the string without finding a ":".
            return false;
        }

        static unsafe bool GetSegmentByName(string segmentName, bool isAlias, out RegSegment segment)
        {
            if (isAlias) return SegmentAliases.TryGetValue(segmentName, out segment);
            else return LoadedSegments.TryGetValue(segmentName, out segment);
        }

        #endregion

        #region Error Throwing Versions

        public static unsafe IRegItem GetItem(string path)
        {
            if (!TryGetItem(path, out IRegItem item))
                ABSLog.ThrowError("T");

            return item;
        }

        public static unsafe void AddItem(string path, IRegItem item)
        {
            if (TryAddItem(path, item) != AddItemResult.Success)
                ABSLog.ThrowError("T");
        }

        public static unsafe void RemoveItem(string path)
        {
            if (!TryRemoveItem(path))
                ABSLog.ThrowError("T");
        }

        public static unsafe RegGroup CreateGroup(string path)
        {
            if (TryCreateGroup(path, out RegGroup newGroup) != AddItemResult.Success)
                ABSLog.ThrowError("T");

            return newGroup;
        }

        #endregion
    }
}
