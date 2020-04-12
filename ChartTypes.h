//#pragma once
//
//// Interface types of the CircularBuffer
////! Representation of a position in 3d space containing three components: x, y and z
//template<typename ElementType_TP>
//class Position3D_TC {
//
//public:
//    Position3D_TC(ElementType_TP x, ElementType_TP y, ElementType_TP z)
//        : _x(x),
//        _y(y),
//        _z(z)
//    {
//    }
//
//    //! x value
//    ElementType_TP _x;
//
//    //! y value
//    ElementType_TP _y;
//
//    //! z value
//    ElementType_TP _z;
//
//    friend std::ostream& operator<<(std::ostream& s, const Position3D_TC& lhs) {
//        s << "Element: " << "x: " << lhs._x
//            << ", y: " << lhs._y
//            << ", z: " << lhs._z
//            << std::endl;
//
//        return s;
//    }
//
//};
//
//
//// Can be used inside the charts to reduce code ( member bars) which is just there for size management inside the chart itself.
//class BoundingBox {
//
//    // TODO:
//private:
//    int _top_left;
//    int _top_right;
//    int _bottom_left;
//    int _bottom_right;
//};