using System;
using System.Collections.Generic;
using System.Text;

namespace ABSoftware.Core.Components
{
    public class ComponentCategory
    {
        public event CategoryChangedEventHandler CategoryChanged;
        List<ComponentList> _components = new List<ComponentList>();

        public ComponentList AddList()
        {
            var newList = new ComponentList();
            _components.Add(newList);

            CategoryChanged?.Invoke(this, new CategoryChangedEventArgs(newList));
            return newList;
        }

        public void RemoveList(int index)
        {
            _components.RemoveAt(index);
            CategoryChanged?.Invoke(this, new CategoryChangedEventArgs(index));
        }

        public ComponentList GetList(int index) => _components[index];

        public IEnumerator<ComponentList> GetEnumerator() => _components.GetEnumerator();
    }

    public enum CategoryChangedType
    {
        Add,
        Remove
    }

    public class CategoryChangedEventArgs : EventArgs
    {
        public CategoryChangedType Type;
        public ComponentList ItemAdded;
        public int ItemRemoved;

        internal CategoryChangedEventArgs(ComponentList added)
        {
            Type = CategoryChangedType.Add;
            ItemAdded = added;
        }

        internal CategoryChangedEventArgs(int itemRemoved)
        {
            Type = CategoryChangedType.Remove;
            ItemRemoved = itemRemoved;
        }
    }

    public delegate void CategoryChangedEventHandler(object sender, CategoryChangedEventArgs e);
}
