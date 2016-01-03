/*
 RGB LED - Automatic Smooth Color Cycling

 Marco Colli
 April 2012
 
 Uses the properties of the RGB Colour Cube
 The RGB colour space can be viewed as a cube of colour. If we assume a cube of dimension 1, then the 
 coordinates of the vertices for the cubve will range from (0,0,0) to (1,1,1) (all black to all white).
 The transitions between each vertex will be a smooth colour flow and we can exploit this by using the 
 path coordinates as the LED transition effect. 
*/
// Output pins for PWM
#define  R_PIN  11  // Red LED
#define  G_PIN  10  // Green LED
#define  B_PIN  9  // Blue LED

// Constants for readability are better than magic numbers
// Used to adjust the limits for the LED, especially if it has a lower ON threshold
#define  MIN_RGB_VALUE  10   // no smaller than 0. 
#define  MAX_RGB_VALUE  255  // no bigger than 255.

// Slowing things down we need ...
#define  TRANSITION_DELAY  70   // in milliseconds, between individual light changes
#define  WAIT_DELAY        500  // in milliseconds, at the end of each traverse
//
// Total traversal time is ((MAX_RGB_VALUE - MIN_RGB_VALUE) * TRANSITION_DELAY) + WAIT_DELAY
// eg, ((255-0)*70)+500 = 18350ms = 18.35s

// Structure to contain a 3D coordinate
typedef struct
{
  byte  x, y, z;
} coord;

static coord  v; // the current rgb coordinates (colour) being displayed

/*
 Vertices of a cube
      
    C+----------+G
    /|        / |
  B+---------+F |
   | |       |  |    y   
   |D+-------|--+H   ^  7 z
   |/        | /     | /
  A+---------+E      +--->x

*/
const coord vertex[] = 
{
 //x  y  z      name
  {0, 0, 0}, // A or 0
  {0, 1, 0}, // B or 1
  {0, 1, 1}, // C or 2
  {0, 0, 1}, // D or 3
  {1, 0, 0}, // E or 4
  {1, 1, 0}, // F or 5
  {1, 1, 1}, // G or 6
  {1, 0, 1}  // H or 7
};

/*
 A list of vertex numbers encoded 2 per byte.
 Hex digits are used as vertices 0-7 fit nicely (3 bits 000-111) and have the same visual
 representation as decimal, so bytes 0x12, 0x34 ... should be interpreted as vertex 1 to 
 v2 to v3 to v4 (ie, one continuous path B to C to D to E).
*/
const byte path[] =
{
  0x01, 0x23, 0x76, 0x54, 0x03, 0x21, 0x56, 0x74,  // trace the edges
  0x13, 0x64, 0x16, 0x02, 0x75, 0x24, 0x35, 0x17, 0x25, 0x70,  // do the diagonals
};

#define  MAX_PATH_SIZE  (sizeof(path)/sizeof(path[0]))  // size of the array

void setup()
{
  pinMode(R_PIN, OUTPUT);   // sets the pins as output
  pinMode(G_PIN, OUTPUT);  
  pinMode(B_PIN, OUTPUT);
}

void traverse(int dx, int dy, int dz)
// Move along the colour line from where we are to the next vertex of the cube.
// The transition is achieved by applying the 'delta' value to the coordinate.
// By definition all the coordinates will complete the transition at the same 
// time as we only have one loop index.
{
  if ((dx == 0) && (dy == 0) && (dz == 0))   // no point looping if we are staying in the same spot!
    return;
    
  for (int i = 0; i < MAX_RGB_VALUE-MIN_RGB_VALUE; i++, v.x += dx, v.y += dy, v.z += dz)
  {
    // set the colour in the LED
    analogWrite(R_PIN, v.x);
    analogWrite(G_PIN, v.y);
    analogWrite(B_PIN, v.z);
    
    delay(TRANSITION_DELAY);  // wait fot the transition delay
  }

  delay(WAIT_DELAY);          // give it an extra rest at the end of the traverse
}

void loop()
{
  int    v1, v2=0;    // the new vertex and the previous one

  // initialise the place we start from as the first vertex in the array
  v.x = (vertex[v2].x ? MAX_RGB_VALUE : MIN_RGB_VALUE);
  v.y = (vertex[v2].y ? MAX_RGB_VALUE : MIN_RGB_VALUE);
  v.z = (vertex[v2].z ? MAX_RGB_VALUE : MIN_RGB_VALUE);

  // Now just loop through the path, traversing from one point to the next
  for (int i = 0; i < 2*MAX_PATH_SIZE; i++)
  {
    // !! loop index is double what the path index is as it is a nybble index !!
    v1 = v2;
    if (i&1)  // odd number is the second element and ...
      v2 = path[i>>1] & 0xf;  // ... the bottom nybble (index /2) or ...
    else      // ... even number is the first element and ...
      v2 = path[i>>1] >> 4;  // ... the top nybble
      
    traverse(vertex[v2].x-vertex[v1].x, 
             vertex[v2].y-vertex[v1].y, 
             vertex[v2].z-vertex[v1].z);
  }
}
