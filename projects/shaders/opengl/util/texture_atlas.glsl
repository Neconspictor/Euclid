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
void toAtlasUvSpace(inout vec2 uv, const in uvec2 tileCount, const in uint tileIndex) 
{
    vec2 scale = vec2(1.0) / vec2(tileCount);
    
    uint rowCount = tileCount.y;
    //uint tileIndexMax = tileCount.x * rowCount - 1;
    //uint newTileIndex = tileIndexMax - tileIndex;
    
    // Get row and column for retrieving the offset
    uint row = tileIndex / rowCount;
    
    //mirror y axis since uv origin is bottom left corner.
    row = (rowCount - 1) - row;
    
    uint column = tileIndex % rowCount; //tileIndex - row * tileCount.x;
    
    vec2 offset = scale * vec2(column, row);
    
    // transform uv coordinate from tile space into atlas space.
    uv = uv * scale + offset;
}

#endif //TEXTURE_ATLAS_HEADER