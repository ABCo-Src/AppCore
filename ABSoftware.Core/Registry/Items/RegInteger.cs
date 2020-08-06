using System;
using System.Collections.Generic;
using System.Text;

namespace ABSoftware.Core.Registry.Items
{
    public class RegInteger : IRegItem
    {
        public event EventHandler ItemChanged;

        int _data;

        public int Data
        {
            get => _data;
            set
            {
                _data = value;
                ItemChanged?.Invoke(this, new EventArgs());
            }
        }
    }
}
