Submarine
=========
(c) 2013 Tiago Rezende

Pair of objects for Pure Data that implement a subjacent stream of data.

Use it when you need to make two pieces of data arrive together at a destination.

------------------------------------------------------------------------

[sub ...]

Submerges data (puts it into the "submerged" stream).
When any trigger hits the left inlet, the data stored through the second inlet (or through the creation parameters) is "submerged", to make it available for a [radar] object to pick up and re-emerge.

[radar]

Re-emerges previously submerged data (recovers it from the "submerged" stream).
When anything reaches the radar's inlet, the last submerged piece of data is emitted through the rightmost outlet, before relaying the inlet's input to the left outlet.

You can nest [sub] and [radar] instances in the same stream, provided you keep them coupled in the same data path.

