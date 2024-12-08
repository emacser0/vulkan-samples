#include "Vertex.h"

bool FVertex::operator==(const FVertex& RHS) const
{
	return Position == RHS.Position && Normal == RHS.Normal && TexCoords == RHS.TexCoords && Tangent == RHS.Tangent;
}
