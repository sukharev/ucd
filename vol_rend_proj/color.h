#ifndef COLOR_H
#define COLOR_H


struct Color {
    float R,G,B;

    Color(void) : R(1), G(1), B(1) {}

    Color(float r,float g,float b) : R(r), G(g), B(b) {}

    Color(const unsigned long &c) {
        B = (c & 0xFF)/255.;
        G = ((c>>8) & 0xFF)/255.;
        R = ((c>>16) & 0xFF)/255.;
    }

    //friend ostream &operator<<(ostream &out, const Color& v);
};

/*
inline ostream &operator<<(ostream &out, const RGBt &c)
{
    out << " [ r=" << c.R << " g=" << c.G << " b=" << c.B << " ] ";
    return out;
}
*/
#endif //COLOR_H
