using System;
using System.Collections.Generic;
using System.Text;

namespace ABSoftware.Core.Registry.Items
{
    public class RegDecimal : IRegItem
    {
        public event EventHandler ItemChanged;

        float _data;
        public float Data
        {
            get => _data;
            set {
                _data = value;
                ItemChanged?.Invoke(this, new EventArgs());
            }
        }
    }
}
