// We dont want any drawing-functions on the serverside, so we define
// these functions as empty functions so that everything compiles.
// Even if we dont use these functions, they will give us linker errors.
void DrawPolygon(struct b2Vec2 const *,int,struct b2Color const &){

}