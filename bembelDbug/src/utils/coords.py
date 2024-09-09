import math

### WIP: porting the DirectedCoord class
class DirectedCoord:

    def __init__(self, x=0,y=0,a=0):
        self.x = x
        self.y = y
        self.a = a

    def toRCS(self, mypos):

        # DirectedCoord out(this->coord.x, this->coord.y, this->angle.rad);
        # // transform from wcs to rcs
        # // 1) POSE
        # // 1a) add translation
        # out.coord.x -= mypos.coord.x;
        # out.coord.y -= mypos.coord.y;
        # // 1b) rotate local coordinates according to alpha!
        # float alpha = -mypos.angle.rad;
        # float cosalpha = cosf(alpha);
        # float sinalpha = sinf(alpha);
        # float x = out.coord.x * cosalpha - out.coord.y * sinalpha;
        # out.coord.y = out.coord.x * sinalpha + out.coord.y * cosalpha;
        # out.coord.x = x;
        # return out;

        out = DirectedCoord(self.x, self.y, self.a)

        out.x -= mypos.x
        out.y -= mypos.y
        
        alpha = -mypos.a

        x = out.x * math.cos(alpha) - out.y * math.sin(alpha)
        out.y = out.x * math.sin(alpha) + out.y * math.cos(alpha)
        out.x = x
        return out

    def walk(self, delta):
        # taken from coords.cpp - walk()
        #####
        # float x1 = this->coord.x;
        # float y1 = this->coord.y;
        # float a1 = this->angle.rad;
        # float x2 = delta.coord.x;
        # float y2 = delta.coord.y;
        # float a2 = delta.angle.rad;
        # DirectedCoord out(0, 0, 0);
        # // Transformation Matrix
        # //out.angle = Angle().normalize( a1 + a2);
        # out.angle.set(a1 + a2);
        # out.coord.x = x1 + cosf(out.angle.rad) * x2 - sinf(out.angle.rad) * y2;
        # out.coord.y = y1 + sinf(out.angle.rad) * x2 + cosf(out.angle.rad) * y2;

        out = DirectedCoord()

        out.a = self.a + delta.a
        out.x = self.x + math.cos(out.a) * delta.x - math.sin(out.a) * delta.y
        out.y = self.y + math.sin(out.a) * delta.x + math.cos(out.a) * delta.y

        return out

    def __add__(self, other): 
        return DirectedCoord(self.x + other.x,self.y + other.y,self.a + other.a)   

    def __sub__(self, other): 
        return DirectedCoord(self.x - other.x,self.y - other.y,self.a - other.a)

    def __iadd__(self, other):
        self.x += other.x
        self.y += other.y
        self.a += other.a

        return self

    def __isub__(self, other):
        self.x -= other.x
        self.y -= other.y
        self.a -= other.a

        return self

    def __str__(self):
        return str(self.x) + "," + str(self.y) + "@" + str(self.a)

    
    def __repr__(self):
        return "DirectedCoord("+str(self.x) + "," + str(self.y) + "," + str(self.a) + ")"

    @classmethod
    def fromBBString(cls, string):
        s = string.split(",")
        s2 = s[1].split("@")
        
        return cls(float(s[0]),float(s2[0]), float(s2[1]))
