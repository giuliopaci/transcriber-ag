// Anchor.h: An anchor is a named offset into signal data
// Author: Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef Anchor_h
#define Anchor_h

#include <ag/AGTypes.h>
#include <ag/Signal.h>


using namespace std;

/**
 * An anchor is a named offset into signal data.
 * @author Xiaoyi Ma
 * @version 1.3 September 21, 2001
 * @note anchors are not required to have an offset;
 * two logically distinct anchors may have the same offset.
 * An anchor with an offset is said to be anchored, otherwise unanchored.
*/
class Anchor {
  friend class AG;
  friend class AnnotationIndex;

 private:
  /// the identifier of this anchor
  const Id id;

  /// a time offset into that timeline
  Offset offset;

  /// the unit of the time offset
  Unit unit;

  /// the signals of the anchor
  set<string> signals;

  /// A boolean value. Set to true if the anchor is anchored, false otherwise.
  bool anchored;

 protected:

  /// Unset the offset.
  void unsetOffset() {
    anchored = false;
    offset = 0.0;
  }

 public:
  /// Create an anchor, using the given identifier
  Anchor(Id id, Offset offset, Unit unit, set<string> const &signals);

  /// Create an anchor, using a given identifier, but without specifying its offset
  Anchor(Id id, Unit unit, set<string> const &signals);

  /// Copy an anchor, but with the given identifier
  Anchor(Id id, const Anchor*);

  /// Get the id of the anchor
  const Id getId() const {return id;}

  /// Get the offset of the anchor
  Offset getOffset() const { return offset; }

  /// Set the offset of the anchor
  void setOffset(Offset o) {offset = o; anchored = true;}

  /// Get the anchored flag of the anchor
  bool getAnchored() const { return anchored; }

  /// Set the signals of the anchor
  void setSignals(set<string>& signals) { this->signals = signals; }

  /// Get the signals of the anchor
  set<string> getSignals() const { return signals; }

  /// Set the unit of the anchor
  void setUnit(string unit) { this->unit = unit; }
  
  /// Get the unit of the anchor
  Unit getUnit() const { return unit; }

  /// Dump the anchor in AIF format.
  string toString();
  /// Comparison of two anchors.
  friend bool operator < (const Anchor&, const Anchor&);
};

/**
 * Anchor compare function.
 * A function to compare anchors by first comparing their offsets,
 * then their ids if the offsets are the same.
 **/
class AnchorCompFunc {
 public:
  bool operator() (const Anchor* a1, const Anchor* a2) const {
    return *a1 < *a2;
  }
};

/**
 * Anchor Seqquence.
 * a set of anchors sorted by AnchorCompFunc, i.e. offsets
 **/
typedef set<Anchor*,AnchorCompFunc> AnchorSeq;
/// AnchorSet
typedef set<Anchor*> AnchorSet;

#endif
