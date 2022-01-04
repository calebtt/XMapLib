using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace XMapLibSharp
{
    public static class StickMapOperations
    {
        public static StickMap Next(this StickMap rhs)
        {
            return rhs == StickMap.LEFT ? StickMap.NEITHER : rhs + 1;
        }

        public static string ToString(this StickMap rhs)
        {
            switch (rhs)
            {
                case StickMap.NEITHER:
                    return "NEITHER";
                case StickMap.LEFT:
                    return "LEFT";
                case StickMap.RIGHT:
                    return "RIGHT";
                default: 
                    return "ERROR";
            }
        }
    }
}
