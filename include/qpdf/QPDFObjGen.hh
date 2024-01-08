// Copyright (c) 2005-2024 Jay Berkenbilt
//
// This file is part of qpdf.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software distributed under the License
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
// or implied. See the License for the specific language governing permissions and limitations under
// the License.
//
// Versions of qpdf prior to version 7 were released under the terms of version 2.0 of the Artistic
// License. At your option, you may continue to consider qpdf to be licensed under those terms.
// Please see the manual for additional information.

#ifndef QPDFOBJGEN_HH
#define QPDFOBJGEN_HH

#include <qpdf/DLL.h>
#include <iostream>
#include <set>

class QPDFObjectHandle;
class QPDFObjectHelper;

// This class represents an object ID and generation pair.  It is suitable to use as a key in a map
// or set.

class QPDFObjGen
{
  public:
    // ABI: change to default.
    QPDF_DLL
    QPDFObjGen()
    {
    }
    QPDF_DLL
    explicit QPDFObjGen(int obj, int gen) :
        obj(obj),
        gen(gen)
    {
    }
    QPDF_DLL
    bool
    operator<(QPDFObjGen const& rhs) const
    {
        return (obj < rhs.obj) || ((obj == rhs.obj) && (gen < rhs.gen));
    }
    QPDF_DLL
    bool
    operator==(QPDFObjGen const& rhs) const
    {
        return (obj == rhs.obj) && (gen == rhs.gen);
    }
    QPDF_DLL
    bool
    operator!=(QPDFObjGen const& rhs) const
    {
        return (obj != rhs.obj) || (gen != rhs.gen);
    }
    QPDF_DLL
    int
    getObj() const
    {
        return obj;
    }
    QPDF_DLL
    int
    getGen() const
    {
        return gen;
    }
    QPDF_DLL
    bool
    isIndirect() const
    {
        return obj != 0;
    }
    QPDF_DLL
    std::string unparse(char separator = ',') const;
    QPDF_DLL
    friend std::ostream& operator<<(std::ostream& os, const QPDFObjGen& og);

    // Convenience class for loop detection when processing objects.
    //
    // The class adds 'add' methods to a std::set<QPDFObjGen> which allows to test whether an
    // QPDFObjGen is present in the set and to insert it in a single operation. The 'add' method is
    // overloaded to take a QPDFObjGen, QPDFObjectHandle or an QPDFObjectHelper as parameter.
    //
    // The erase method is modified to ignore requests to erase QPDFObjGen(0, 0).
    //
    // Usage example:
    //
    // void process_object(QPDFObjectHandle oh, QPDFObjGen::set& seen)
    // {
    //     if (seen.add(oh)) {
    //         // handle first encounter of oh
    //     } else {
    //         // handle loop / subsequent encounter of oh
    //     }
    // }
    class QPDF_DLL_CLASS set: public std::set<QPDFObjGen>
    {
      public:
        // Add 'og' to the set. Return false if 'og' is already present in the set. Attempts to
        // insert QPDFObjGen(0, 0) are ignored.
        QPDF_DLL
        bool
        add(QPDFObjGen og)
        {
            if (og.isIndirect()) {
                if (count(og) > 0) {
                    return false;
                }
                emplace(og);
            }
            return true;
        }

        QPDF_DLL
        bool add(QPDFObjectHandle const& oh);

        QPDF_DLL
        bool add(QPDFObjectHelper const& oh);

        QPDF_DLL
        void
        erase(QPDFObjGen og)
        {
            if (og.isIndirect()) {
                std::set<QPDFObjGen>::erase(og);
            }
        }

        QPDF_DLL
        void erase(QPDFObjectHandle const& oh);

        QPDF_DLL
        void erase(QPDFObjectHelper const& oh);
    };

  private:
    // This class does not use the Members pattern to avoid a memory allocation for every one of
    // these. A lot of these get created and destroyed.
    int obj{0};
    int gen{0};
};

#endif // QPDFOBJGEN_HH
