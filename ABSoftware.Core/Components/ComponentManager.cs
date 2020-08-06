using ABSoftware.Core.Logging;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Text;

namespace ABSoftware.Core.Components
{

    public static class ComponentManager
    {
        internal static Dictionary<string, ComponentCategory> Components = new Dictionary<string, ComponentCategory>();

        public static ComponentCategory AddCategory(string categoryName)
        {
            var category = new ComponentCategory();
            Components.Add(categoryName, category);
            return category;
        }

        public static void RemoveCategory(string categoryName) => Components.Remove(categoryName);
        public static bool CategoryExists(string categoryName) => Components.ContainsKey(categoryName);
        public static bool TryGetCategory(string categoryName, out ComponentCategory res) => Components.TryGetValue(categoryName, out res);

        public static ComponentCategory GetCategory(string categoryName)
        {
            if (Components.TryGetValue(categoryName, out ComponentCategory res))
                return res;

            ABSLog.ThrowError("An attempt was made to access a non-existant component category. Always use 'CategoryExists' or 'TryGetCategory' if the category is not guaranteed to exist.");
            return null;
        }

        public static ComponentList GetList(string categoryName, int index) => GetCategory(categoryName).GetList(index);
    }
}
