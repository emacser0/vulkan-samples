#include "Vertex.h"

bool FColorVertex::operator==(const FColorVertex& RHS) const
{
	return Position == RHS.Position && Color == RHS.Color && TexCoord == RHS.TexCoord;
}

bool FVertex::operator==(const FVertex& RHS) const
{
	return Position == RHS.Position && Normal == RHS.Normal && TexCoord == RHS.TexCoord;
}

bool FTangentVertex::operator==(const FTangentVertex& RHS) const
{
	return Position == RHS.Position && Normal == RHS.Normal && TexCoord == RHS.TexCoord && Tangent == RHS.Tangent;
}
