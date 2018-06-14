#ifndef CZI_NANOPORE2_MARKER_ID_HPP
#define CZI_NANOPORE2_MARKER_ID_HPP

#include "cstdint.hpp"

namespace ChanZuckerberg {
    namespace Nanopore2 {

        // Type used to globally identify a marker on an oriented read.
        // This is the global index of the marker in Assembler::markers.
        // For a human assembly with coverage 40X the total number
        // of markers is more than 20 billions (counting both strands),
        // so this needs to be uint64_t. There could, however, be situations
        // where uint32_t is sufficient.
        using MarkerId = uint64_t;

        // Type used to identify a vertex of the global marker graph.
        using GlobalMarkerGraphVertexId = MarkerId;

    }
}

#endif
