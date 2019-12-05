#ifndef TEXTURE_ATLAS_HEADER
#define TEXTURE_ATLAS_HEADER

/**
 * Transforms a uv coordinate in texture tile space into texture atlas space.
 *
 * @param uv        : UV coordinate in range [0, 1] that should be transformed for a tile in the texture atlas uv space.
 *                    The transformed result will be written into this parameter.
 * @param tileCount : Specifies the number of tiles in x and y axis direction of the texture atlas.
 * @param tileIndex : The index of the tile for which the uv coordinates should be transformed. 
 */
void toAtlasUvSpace(inout vec2 uv, const in ivec2 tileCount, const in int tileIndex) 
{
    vec2 scale = vec2(1.0) / vec2(tileCount);
    
    // Get row and column for retrieving the offset
    int row = tileIndex / tileCount.y;
    int column = tileIndex - row * tileCount.y;
    vec2 offset = scale * vec2(row, column);
    
    // transform uv coordinate from tile space into atlas space.
    uv = uv * scale + offset;
}

#endif //TEXTURE_ATLAS_HEADER