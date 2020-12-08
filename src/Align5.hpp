#ifndef SHASTA_ALIGN5_HPP
#define SHASTA_ALIGN5_HPP

/*******************************************************************************

Marker alignment of two sequences markerSequence0 and markerSequence1,
each defined as a sequence of marker KmerId's.

We call x or y the index (or position or ordinal)
of a marker in markerSequence0 or markerSequence1 respectively, so:
    - markerSequence0[x] is the marker at position x of markerSequence0
    - markerSequence1[y] is the marker at position y of markerSequence1

The number of markers in markerSequence0 is nx
and the number of markers in markerSequence1 is ny.
For any two positions x and y the following hold:
0 <= x <= nx-1
0 <= y <= ny-1

We also consider Feature's which are sequences of m markers
in each of the two sequences, where m is the template parameter of
class Aligner.

So for example consider an input sequence
consisting of the following marker KmerId's:
45 58 106 17
If m=2, the sequence of Feature's representing this sequence is
(45,58) (58,106), (106,17).

The two sequences of Feature's corresponding to
markerSequence0 and markerSequence1
are featureSequence0 and featureSequence1.

Note that the sequences of Feature's are shorter (by m-1)
than the original marker sequences.

The alignment matrix in feature space is sparse because of the
large alphabet. For example, with default options there are
about 8000 marker KmerId's and therefore about 64000000
distinct features for m=2.

The coordinates in the alignment matrix in marker or feature space
are x and y, with x
represented along the horizontal axis and increasing toward the right,
and y represented along the vertical axis and increasing toward
the bottom. The alignment matrix element at position (x,y)
exists if markerSequence0[x]=markerSequence1[y] when
working with sequences of markers and if
featureSequence0[x]=featureSequence1[y] when working with sequences
of features.

We also use coordinates X and Y defined as:
X = x + y
Y = y + (nx - 1 - x)

It can be verified that:
0 <= X <= nx + ny - 2
0 <= Y <= nx + ny - 2
So the total number of disntict values of X and Y is nx + ny - 1.

X is a coordinate along the diagonal of the alignment matrix,
and Y is orthogonal to it and identifies the diagonal.
In (X,Y) coordinates the alignment matrix is a subset of
the square of size nx + ny -1. The alignment matrix is
rotated by 45 degrees relative to this square.

We use a sparse representation of the alignment matrix
in which non-zero alignment matrix entries are stored
organized by cell in a rectangular arrangement if cells
of size (deltaX, deltaY) in (X,Y) space .

*******************************************************************************/

#include "hashArray.hpp"
#include "Marker.hpp"
#include "MemoryMappedVectorOfVectors.hpp"
#include "span.hpp"

#include "array.hpp"
#include <limits>
#include <unordered_map>
#include "utility.hpp"
#include "vector.hpp"

namespace shasta {
    class Alignment;
    class AlignmentInfo;
    class PngImage;

    namespace Align5 {
        template<uint64_t m> class Aligner;
        class MatrixEntry;
        class Options;

        // This is used to store (x,y), (X,Y), or (iX, iY).
        using Coordinates = pair<uint32_t, uint32_t>;
    }

    void align5(
        const span<const CompressedMarker>&,
        const span<const CompressedMarker>&,
        const Align5::Options&,
        MemoryMapped::VectorOfVectors<Align5::MatrixEntry, uint64_t>&, // Used as work area.
        Alignment&,
        AlignmentInfo&,
        bool debug);

    template<uint64_t m> void align5(
        const span<const CompressedMarker>&,
        const span<const CompressedMarker>&,
        const Align5::Options&,
        MemoryMapped::VectorOfVectors<Align5::MatrixEntry, uint64_t>&, // Used as work area.
        Alignment&,
        AlignmentInfo&,
        bool debug);

}



class shasta::Align5::Options {
public:
    uint64_t m;
    uint64_t deltaX;
    uint64_t deltaY;
    int64_t matchScore;
    int64_t mismatchScore;
    int64_t gapScore;
};



class shasta::Align5::MatrixEntry {
public:
    Coordinates xy;
    MatrixEntry() {}
    MatrixEntry(const Coordinates& xy) : xy(xy) {}
};



template<uint64_t m> class shasta::Align5::Aligner {
public:

    using MarkerSequence = span<const CompressedMarker>;

    // The constructor does all the work.
    Aligner(
        const MarkerSequence&,
        const MarkerSequence&,
        const Options&,
        MemoryMapped::VectorOfVectors<MatrixEntry, uint64_t>& matrix, // Used as work area.
        Alignment&,
        AlignmentInfo&,
        bool debug);

private:

    // Number of markers (not features) in the two sequences being aligned.
    uint32_t nx;
    uint32_t ny;

    // Cell sizes in the X and Y direction.
    uint32_t deltaX;
    uint32_t deltaY;

    // For each sequence, vectors of pairs (Feature, ordinal)
    // sorted by feature.
    using Feature = array<KmerId, m>;
    vector< pair<Feature, uint32_t> > sortedFeatures0;
    vector< pair<Feature, uint32_t> > sortedFeatures1;
    static void sortFeatures(
        const MarkerSequence&,
        vector< pair<Feature, uint32_t> >&);

    // For each sequence, vectors of pairs (markerId, ordinal)
    // sorted by marker id.
    vector< pair<KmerId, uint32_t> > sortedMarkers0;
    vector< pair<KmerId, uint32_t> > sortedMarkers1;
    static void sortMarkers(
        const MarkerSequence&,
        vector< pair<KmerId, uint32_t> >& sortedMarkers);

    // Write the alignment matrix in marker space or feature space
    // to a png image.
    void writeAlignmentMatrixInMarkerSpace(const string& fileName) const;
    void writeAlignmentMatrixInFeatureSpace(const string& fileName) const;
    void writeCheckerboard(PngImage&) const;



    // The alignment matrix, in a sparse representation organized by
    // cells in (X,Y) space.
    // For each iY, we store pairs(iX, xy) sorted by iX.
    // Even though this requires sorting, it is more efficient
    // than using a hash table, due to the better memory access pattern.
    vector< vector< pair<uint32_t, Coordinates> > > alignmentMatrix;
    void createAlignmentMatrix();
    void writeAlignmentMatrix(const string& fileName) const;



    // Cells in (X,Y) space.
    // Stored similarly to alignmentMatrix above: for each iY,
    // we store pairs (iX, Cell) sorted by iX.
    class Cell {
    public:
    };
    vector< vector< pair<uint32_t, Cell> > > cells;
    void createCells(uint32_t minEntryCountPerCell);
    void writeCellsCsv(const string& fileName) const;
    void writeCellsPng(const string& fileName) const;



    // Coordinate transformations.

    // Return (X,Y) given (x,y).
    Coordinates getXY(Coordinates xy) const;

    // Return (iX,iY) given (X,Y).
    Coordinates getCellIndexesFromXY(Coordinates XY) const;

    // Return (iX,iY) given (xy).
    Coordinates getCellIndexesFromxy(Coordinates xy) const;

};



#endif