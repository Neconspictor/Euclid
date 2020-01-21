#ifndef CUBEMAP_H
#define CUBEMAP_H

/**
 * Produces a direction vector of a uv coordinate from a given cubemap side.
 * Inspired by  
 * 'Mathemathics for 3D Game Programming and Computer Graphics', 3rd edition, Eric Lengyel ,p. 170
 */
vec3 getDirection(int side, in vec2 uv) {
switch(side) {
		case 0: // positive X
			return vec3(1.0, -2.0 * uv.y + 1.0, -2.0 * uv.x + 1.0);
		case 1: // negative X
		return vec3(-1.0, -2.0 * uv.y + 1.0, 2.0 * uv.x - 1.0);
		case 2: // positive Y
		return vec3(2.0 * uv.x - 1.0, 1.0, 2.0 * uv.y - 1.0);
		case 3: // negative Y
		return vec3(2.0 * uv.x - 1.0, -1.0, -2.0 * uv.y + 1.0);
		case 4: // positive Z
		return vec3(2.0 * uv.x - 1.0, -2.0 * uv.y + 1.0, 1);
		case 5: // negative Z
		return vec3(-2.0 * uv.x + 1.0, -2.0 * uv.y + 1.0, -1);
	}
}

#endif //CUBEMAP_H