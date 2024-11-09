#include "Vertex.h"

bool FVertex::operator==(const FVertex& RHS) const
{
	return Position == RHS.Position && Color == RHS.Color && TexCoords == RHS.TexCoords;
}
