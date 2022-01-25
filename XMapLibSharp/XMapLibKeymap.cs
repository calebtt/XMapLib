using System;
using System.Windows.Forms;

namespace XMapLibSharp
{
    public struct XMapLibKeymap
    {
        private int _vkMappedTo;
        private int _vkMappedFrom;

        public int VkMappedFrom
        {
            get => _vkMappedFrom;
            set
            {
                _vkMappedFrom = value;
                string? result = Enum.GetName((ControllerButtons)_vkMappedFrom);
                if (result != null)
                    VkMappedFromAka = result;
            }
        }
        public string VkMappedFromAka { get; set; }
        public int VkMappedTo
        {
            get => _vkMappedTo;
            set
            {
                _vkMappedTo = value;
                string? result = Enum.GetName((Keys)_vkMappedTo);
                if (result != null)
                    VkMappedToAka = result;
            }
        }
        public string VkMappedToAka {  get; set; }
        public bool UsesRepeatBehavior { get; set; }

        /////<summary>Copy constructor</summary>
        //public XMapLibKeymap(XMapLibKeymap other)
        //{
        //    _vkMappedTo = 0;
        //    _vkMappedFrom = 0;
        //    VkMappedFromAka = "";
        //    VkMappedToAka = "";
        //    VkMappedFrom = other.VkMappedFrom;
        //    VkMappedTo = other.VkMappedTo;
        //    UsesRepeatBehavior = other.UsesRepeatBehavior;
        //}
    }
}
