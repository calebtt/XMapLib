using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace XMapLibSharp
{
    public static class XMapLibStickMapOperations
    {
        public static XMapLibStickMap Next(this XMapLibStickMap rhs)
        {
            return rhs == XMapLibStickMap.LEFT ? XMapLibStickMap.NEITHER : rhs + 1;
        }

        public static string ToString(this XMapLibStickMap rhs)
        {
            switch (rhs)
            {
                case XMapLibStickMap.NEITHER:
                    return "NEITHER";
                case XMapLibStickMap.LEFT:
                    return "LEFT";
                case XMapLibStickMap.RIGHT:
                    return "RIGHT";
                default: 
                    return "ERROR";
            }
        }
    }
}
