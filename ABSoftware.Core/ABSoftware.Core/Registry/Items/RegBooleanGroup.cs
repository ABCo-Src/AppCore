using System;
using System.Collections.Generic;
using System.Text;

namespace ABSoftware.Core.Registry.Items
{
    public delegate void BooleanItemChangedEventHandler(object sender, BooleanItemChangedEventArgs args);

    public class BooleanItemChangedEventArgs : EventArgs
    {
        public int Pos;

        public BooleanItemChangedEventArgs(int pos) => Pos = pos;
    }

    public class RegBooleanGroup : IRegItem
    {
        public event EventHandler ItemChanged;
        public event BooleanItemChangedEventHandler BooleanItemChanged;

        int _data;

        public bool GetItem(byte pos) => (_data & (1 << pos)) > 0;

        public void SetItem(byte pos, bool val) {
            int mask = 1 << pos;

            if (val) _data |= mask;
            else _data &= ~mask;

            ItemChanged?.Invoke(this, new EventArgs());
            BooleanItemChanged?.Invoke(this, new BooleanItemChangedEventArgs(pos));
        }
    }
}
