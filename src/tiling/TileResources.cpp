
#include "TileResources.h"
#include "rendering/MeshBuffer.h"

unsigned int TileResources::vao() const {
    return _meshBuffer->vao();
}
