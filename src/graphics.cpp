//
//  graphics.cpp
//  Game Engine
//

#include <stb_image.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "core.hpp"

using namespace tinyobj;


// MARK: -
// MARK: Properties
const vector<float> & GraphicsComponent::vertices() const
{
  return _verts;
}
int GraphicsComponent::numberOfVertices() const
{
  return (int)_verts.size()/3;
}
bool GraphicsComponent::hasDiffuseTexture() const
{
  return _hasDiffTex;
}
const vec3 & GraphicsComponent::diffuseColor() const
{
  return _diffCol;
}
bool GraphicsComponent::deferredShading() const
{
  return _deferShading;
}

// MARK: Member functions
GraphicsComponent::GraphicsComponent()
  : Component()
  , _hasDiffTex(false)
  , _diffCol(0.8, 0.8, 0.8)
  , _deferShading(false)
{}

void GraphicsComponent::init(Entity * entity)
{
  Component::init(entity);
  
  glGenVertexArrays(1, &_vao);
}

void GraphicsComponent::render(const Core & core)
{
  // activate textures
  if (_hasDiffTex)
  {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _colTexMap);
  }
  
  // render
  glBindVertexArray(_vao);
  glDrawArrays(GL_TRIANGLES, 0, (int)_verts.size());
}

bool GraphicsComponent::loadObject(const char * fileName)
{
  attrib_t           attrib;
  vector<shape_t>    shapes;
  string             error;
  LoadObj(&attrib, &shapes, nullptr, &error, fileName);

  if (shapes.size() >= 1 && !attrib.vertices.empty())
  {
    // initialize data collections
    vector<float> normals, texCoords;
    auto shape   = shapes[0];
    int numVerts = (int)shape.mesh.indices.size();
    _verts.resize(3*numVerts);
    normals.resize(3*numVerts);
    texCoords.resize(2*numVerts);
    
    // populate data collections
    for (int face = 0; face < numVerts/3; ++face)
    {
      // indices
      const auto i0 = shape.mesh.indices[3*face];
      const auto i1 = shape.mesh.indices[3*face+1];
      const auto i2 = shape.mesh.indices[3*face+2];
      
      // vertices
      const vec3 v0 =
      {
        attrib.vertices[3*i0.vertex_index],
        attrib.vertices[3*i0.vertex_index+1],
        attrib.vertices[3*i0.vertex_index+2]
      };
      const vec3 v1 =
      {
        attrib.vertices[3*i1.vertex_index],
        attrib.vertices[3*i1.vertex_index+1],
        attrib.vertices[3*i1.vertex_index+2]
      };
      const vec3 v2 =
      {
        attrib.vertices[3*i2.vertex_index],
        attrib.vertices[3*i2.vertex_index+1],
        attrib.vertices[3*i2.vertex_index+2]
      };
      
      _verts[9*face]   = v0.x;
      _verts[9*face+1] = v0.y;
      _verts[9*face+2] = v0.z;
      _verts[9*face+3] = v1.x;
      _verts[9*face+4] = v1.y;
      _verts[9*face+5] = v1.z;
      _verts[9*face+6] = v2.x;
      _verts[9*face+7] = v2.y;
      _verts[9*face+8] = v2.z;
      
      // normals
      if (!attrib.normals.empty())
      {
        normals[9*face]   = attrib.normals[3*i0.normal_index];
        normals[9*face+1] = attrib.normals[3*i0.normal_index+1];
        normals[9*face+2] = attrib.normals[3*i0.normal_index+2];
        normals[9*face+3] = attrib.normals[3*i1.normal_index];
        normals[9*face+4] = attrib.normals[3*i1.normal_index+1];
        normals[9*face+5] = attrib.normals[3*i1.normal_index+2];
        normals[9*face+6] = attrib.normals[3*i2.normal_index];
        normals[9*face+7] = attrib.normals[3*i2.normal_index+1];
        normals[9*face+8] = attrib.normals[3*i2.normal_index+2];
      }
      else
      {
        // calculate face normal
        vec3 e0 = normalize(v1-v0);
        vec3 e1 = normalize(v2-v0);
        vec3 n  = cross(e0, e1);
        normals[9*face]   = n.x;
        normals[9*face+1] = n.y;
        normals[9*face+2] = n.z;
        normals[9*face+3] = n.x;
        normals[9*face+4] = n.y;
        normals[9*face+5] = n.z;
        normals[9*face+6] = n.x;
        normals[9*face+7] = n.y;
        normals[9*face+8] = n.z;
      }
      
      // texture coordinates
      texCoords[6*face]   = attrib.texcoords[2*i0.texcoord_index];
      texCoords[6*face+1] = attrib.texcoords[2*i0.texcoord_index+1];
      texCoords[6*face+2] = attrib.texcoords[2*i1.texcoord_index];
      texCoords[6*face+3] = attrib.texcoords[2*i1.texcoord_index+1];
      texCoords[6*face+4] = attrib.texcoords[2*i2.texcoord_index];
      texCoords[6*face+5] = attrib.texcoords[2*i2.texcoord_index+1];
    }
    
    // bind vertex array object
    glBindVertexArray(_vao);
    
    // generate vertices buffer
    GLuint vertsBuf;
    glGenBuffers(1, &vertsBuf);
    glBindBuffer(GL_ARRAY_BUFFER, vertsBuf);
    glBufferData(GL_ARRAY_BUFFER, 3*numVerts*sizeof(float), &_verts[0],
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(0);
    
    // generate normals buffer
    GLuint normalsBuf;
    glGenBuffers(1, &normalsBuf);
    glBindBuffer(GL_ARRAY_BUFFER, normalsBuf);
    glBufferData(GL_ARRAY_BUFFER, 3*numVerts*sizeof(float), &normals[0],
                 GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(1);
    
    // generate texture coordinates buffer
    GLuint texCoordsBuf;
    glGenBuffers(1, &texCoordsBuf);
    glBindBuffer(GL_ARRAY_BUFFER, texCoordsBuf);
    glBufferData(GL_ARRAY_BUFFER, 2*numVerts*sizeof(float), &texCoords[0],
                 GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(2);
    
    return true;
  }
  return false;
}

bool GraphicsComponent::loadTexture(const char * fileName, TextureType texType)
{
  // load image
  int w, h, comps;
  auto image = stbi_load(fileName, &w, &h, &comps, STBI_rgb_alpha);
  if (image) {
    // determine texture type
    GLuint * texture = nullptr;
    if (texType == Diffuse)
    {
      texture     = &_colTexMap;
      _hasDiffTex = true;
    }
    else
      // TODO: implement more texture types
      return nullptr;
    
    // generate texture
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, image);
    free(image);
    
    // set filtering options and attach texture to shader
    if (texType == Diffuse)
      glGenerateMipmap(GL_TEXTURE_2D);
    else
    {
      // other texture types
    }
    
    return true;
  }
  return false;
}

bool GraphicsComponent::loadShader(const char * vsfn, const char * fsfn)
{
  // TODO: implement
  return false;
}

string GraphicsComponent::trait() const { return "Graphics"; }

void GraphicsComponent::diffuseColor(const vec3 & col)
{
  _diffCol = col;
}

void GraphicsComponent::deferredShading(bool enabled)
{
  _deferShading = enabled;
}
