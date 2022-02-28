#pragma once
# include <Siv3D.hpp> // OpenSiv3D v0.4.2

Point GetPos(const FilePath& tomlPath)
{
	const TOMLReader toml(tomlPath);

	Point pos = { 0,0 };

	if (!toml)
	{
		return pos;
	}

	pos = { toml[U"point.pos.x"].get<int32>(),toml[U"point.pos.y"].get<int32>() };
	return pos;

}

double GetGravity(const FilePath& tomlPath)
{
	const TOMLReader toml(tomlPath);

	double g = 0;
	if (!toml)
	{
		return g;
	}

	g = toml[U"gravity.g"].get<int32>();
	return g;

}

int32 GetError(const FilePath& tomlPath)
{
	const TOMLReader toml(tomlPath);

	int32 r = 0;

	if (!toml)
	{
		return r;
	}

	r = toml[U"error.e"].get<int32>();
	return r;

}

int32 GetRadius(const FilePath& tomlPath)
{
	const TOMLReader toml(tomlPath);

	int32 r = 0;

	if (!toml)
	{
		return r;
	}

	r = toml[U"radius.r"].get<int32>();
	return r;

}