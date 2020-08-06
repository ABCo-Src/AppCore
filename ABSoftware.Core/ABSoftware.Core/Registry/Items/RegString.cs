using System;
using System.Collections.Generic;
using System.Text;

namespace ABSoftware.Core.Registry.Items
{
    public class RegString : IRegItem
    {
        public event EventHandler ItemChanged;

        string _data;

        public string Data
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
