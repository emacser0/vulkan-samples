#pragma once

class FWidget
{
public:
	FWidget();
	virtual ~FWidget();

	virtual void Draw() = 0;

private:
};
