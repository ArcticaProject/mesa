/* $Id: varray.c,v 1.1.1.1 1999/08/19 00:55:41 jtg Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  3.1
 * 
 * Copyright (C) 1999  Brian Paul   All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifdef PC_HEADER
#include "all.h"
#else
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "context.h"
#include "api.h"
#include "cva.h"
#include "enable.h"
#include "enums.h"
#include "dlist.h"
#include "light.h"
#include "macros.h"
#include "mmath.h"
#include "pipeline.h"
#include "texstate.h"
#include "translate.h"
#include "types.h"
#include "varray.h"
#include "vb.h"
#include "vbfill.h"
#include "vbrender.h"
#include "vbindirect.h"
#include "vbxform.h"
#include "xform.h"
#ifdef XFree86Server
#include "GL/xf86glx.h"
#endif
#endif


void GLAPIENTRY glVertexPointer(CTX_ARG GLint size, GLenum type, GLsizei stride,
                                 const GLvoid *ptr )
{
   GLcontext *ctx;
   GET_CONTEXT;
   CHECK_CONTEXT;
   ctx = CC;
   
   if (size<2 || size>4) {
      gl_error( ctx, GL_INVALID_VALUE, "glVertexPointer(size)" );
      return;
   }
   if (stride<0) {
      gl_error( ctx, GL_INVALID_VALUE, "glVertexPointer(stride)" );
      return;
   }
   
   if (MESA_VERBOSE&(VERBOSE_VARRAY|VERBOSE_API))
      fprintf(stderr, "glVertexPointer( sz %d type %s stride %d )\n", size, 
	      gl_lookup_enum_by_nr( type ),
	      stride);

   ctx->Array.Vertex.StrideB = stride;
   if (!stride) {
      switch (type) {
      case GL_SHORT:
         ctx->Array.Vertex.StrideB =  size*sizeof(GLshort);
         break;
      case GL_INT:
         ctx->Array.Vertex.StrideB =  size*sizeof(GLint);
         break;
      case GL_FLOAT:
         ctx->Array.Vertex.StrideB =  size*sizeof(GLfloat);
         break;
      case GL_DOUBLE:
         ctx->Array.Vertex.StrideB =  size*sizeof(GLdouble);
         break;
      default:
         gl_error( ctx, GL_INVALID_ENUM, "glVertexPointer(type)" );
         return;
      }
   }
   ctx->Array.Vertex.Size = size;
   ctx->Array.Vertex.Type = type;
   ctx->Array.Vertex.Stride = stride;
   ctx->Array.Vertex.Ptr = (void *) ptr;
   ctx->Array.VertexFunc = gl_trans_4f_tab[size][TYPE_IDX(type)];
   ctx->Array.VertexEltFunc = gl_trans_elt_4f_tab[size][TYPE_IDX(type)];
   ctx->Array.NewArrayState |= VERT_OBJ_ANY;
   ctx->NewState |= NEW_CLIENT_STATE;
}




void GLAPIENTRY glNormalPointer(CTX_ARG GLenum type, GLsizei stride,
                                 const GLvoid *ptr )
{
   GLcontext *ctx;
   GET_CONTEXT;
   CHECK_CONTEXT;
   ctx = CC;
   
   if (stride<0) {
      gl_error( ctx, GL_INVALID_VALUE, "glNormalPointer(stride)" );
      return;
   }

   if (MESA_VERBOSE&(VERBOSE_VARRAY|VERBOSE_API))
      fprintf(stderr, "glNormalPointer( type %s stride %d )\n", 
	      gl_lookup_enum_by_nr( type ),
	      stride);

   ctx->Array.Normal.StrideB = stride;
   if (!stride) {
      switch (type) {
      case GL_BYTE:
         ctx->Array.Normal.StrideB =  3*sizeof(GLbyte);
         break;
      case GL_SHORT:
         ctx->Array.Normal.StrideB =  3*sizeof(GLshort);
         break;
      case GL_INT:
         ctx->Array.Normal.StrideB =  3*sizeof(GLint);
         break;
      case GL_FLOAT:
         ctx->Array.Normal.StrideB =  3*sizeof(GLfloat);
         break;
      case GL_DOUBLE:
         ctx->Array.Normal.StrideB =  3*sizeof(GLdouble);
         break;
      default:
         gl_error( ctx, GL_INVALID_ENUM, "glNormalPointer(type)" );
         return;
      }
   }
   ctx->Array.Normal.Type = type;
   ctx->Array.Normal.Stride = stride;
   ctx->Array.Normal.Ptr = (void *) ptr;
   ctx->Array.NormalFunc = gl_trans_3f_tab[TYPE_IDX(type)];
   ctx->Array.NormalEltFunc = gl_trans_elt_3f_tab[TYPE_IDX(type)];
   ctx->Array.NewArrayState |= VERT_NORM;
   ctx->NewState |= NEW_CLIENT_STATE;
}



void GLAPIENTRY glColorPointer(CTX_ARG GLint size, GLenum type, GLsizei stride,
                                const GLvoid *ptr )
{
   GLcontext *ctx;
   GET_CONTEXT;
   CHECK_CONTEXT;
   ctx = CC;
   if (size<3 || size>4) {
      gl_error( ctx, GL_INVALID_VALUE, "glColorPointer(size)" );
      return;
   }
   if (stride<0) {
      gl_error( ctx, GL_INVALID_VALUE, "glColorPointer(stride)" );
      return;
   }

   if (MESA_VERBOSE&(VERBOSE_VARRAY|VERBOSE_API))
      fprintf(stderr, "glColorPointer( sz %d type %s stride %d )\n", size, 
	  gl_lookup_enum_by_nr( type ),
	  stride);

   ctx->Array.Color.StrideB = stride;
   if (!stride) {
      switch (type) {
      case GL_BYTE:
         ctx->Array.Color.StrideB =  size*sizeof(GLbyte);
         break;
      case GL_UNSIGNED_BYTE:
         ctx->Array.Color.StrideB =  size*sizeof(GLubyte);
         break;
      case GL_SHORT:
         ctx->Array.Color.StrideB =  size*sizeof(GLshort);
         break;
      case GL_UNSIGNED_SHORT:
         ctx->Array.Color.StrideB =  size*sizeof(GLushort);
         break;
      case GL_INT:
         ctx->Array.Color.StrideB =  size*sizeof(GLint);
         break;
      case GL_UNSIGNED_INT:
         ctx->Array.Color.StrideB =  size*sizeof(GLuint);
         break;
      case GL_FLOAT:
         ctx->Array.Color.StrideB =  size*sizeof(GLfloat);
         break;
      case GL_DOUBLE:
         ctx->Array.Color.StrideB =  size*sizeof(GLdouble);
         break;
      default:
         gl_error( ctx, GL_INVALID_ENUM, "glColorPointer(type)" );
         return;
      }
   }
   ctx->Array.Color.Size = size;
   ctx->Array.Color.Type = type;
   ctx->Array.Color.Stride = stride;
   ctx->Array.Color.Ptr = (void *) ptr;
   ctx->Array.ColorFunc = gl_trans_4ub_tab[size][TYPE_IDX(type)];
   ctx->Array.ColorEltFunc = gl_trans_elt_4ub_tab[size][TYPE_IDX(type)];
   ctx->Array.NewArrayState |= VERT_RGBA;
   ctx->NewState |= NEW_CLIENT_STATE;
}



void GLAPIENTRY glIndexPointer(CTX_ARG GLenum type, GLsizei stride,
                                const GLvoid *ptr )
{
   GLcontext *ctx;
   GET_CONTEXT;
   CHECK_CONTEXT;
   ctx = CC;
   
   if (stride<0) {
      gl_error( ctx, GL_INVALID_VALUE, "glIndexPointer(stride)" );
      return;
   }

   ctx->Array.Index.StrideB = stride;
   if (!stride) {
      switch (type) {
      case GL_UNSIGNED_BYTE:
         ctx->Array.Index.StrideB =  sizeof(GLubyte);
         break;
      case GL_SHORT:
         ctx->Array.Index.StrideB =  sizeof(GLshort);
         break;
      case GL_INT:
         ctx->Array.Index.StrideB =  sizeof(GLint);
         break;
      case GL_FLOAT:
         ctx->Array.Index.StrideB =  sizeof(GLfloat);
         break;
      case GL_DOUBLE:
         ctx->Array.Index.StrideB =  sizeof(GLdouble);
         break;
      default:
         gl_error( ctx, GL_INVALID_ENUM, "glIndexPointer(type)" );
         return;
      }
   }
   ctx->Array.Index.Type = type;
   ctx->Array.Index.Stride = stride;
   ctx->Array.Index.Ptr = (void *) ptr;
   ctx->Array.IndexFunc = gl_trans_1ui_tab[TYPE_IDX(type)];
   ctx->Array.IndexEltFunc = gl_trans_elt_1ui_tab[TYPE_IDX(type)];
   ctx->Array.NewArrayState |= VERT_INDEX;
   ctx->NewState |= NEW_CLIENT_STATE;
}



void GLAPIENTRY glTexCoordPointer(CTX_ARG GLint size, GLenum type,
                                   GLsizei stride, const GLvoid *ptr )
{
   GLuint texUnit;
   
   GLcontext *ctx;
   GET_CONTEXT;
   CHECK_CONTEXT;
   ctx = CC;
   
   texUnit = ctx->TexCoordUnit;

   if (size<1 || size>4) {
      gl_error( ctx, GL_INVALID_VALUE, "glTexCoordPointer(size)" );
      return;
   }
   if (stride<0) {
      gl_error( ctx, GL_INVALID_VALUE, "glTexCoordPointer(stride)" );
      return;
   }

   if (MESA_VERBOSE&(VERBOSE_VARRAY|VERBOSE_API))
      fprintf(stderr, "glTexCoordPointer( unit %u sz %d type %s stride %d )\n", 
	  texUnit,
	  size, 
	  gl_lookup_enum_by_nr( type ),
	  stride);

   ctx->Array.TexCoord[texUnit].StrideB = stride;
   if (!stride) {
      switch (type) {
      case GL_SHORT:
         ctx->Array.TexCoord[texUnit].StrideB =  size*sizeof(GLshort);
         break;
      case GL_INT:
         ctx->Array.TexCoord[texUnit].StrideB =  size*sizeof(GLint);
         break;
      case GL_FLOAT:
         ctx->Array.TexCoord[texUnit].StrideB =  size*sizeof(GLfloat);
         break;
      case GL_DOUBLE:
         ctx->Array.TexCoord[texUnit].StrideB =  size*sizeof(GLdouble);
         break;
      default:
         gl_error( ctx, GL_INVALID_ENUM, "glTexCoordPointer(type)" );
         return;
      }
   }
   ctx->Array.TexCoord[texUnit].Size = size;
   ctx->Array.TexCoord[texUnit].Type = type;
   ctx->Array.TexCoord[texUnit].Stride = stride;
   ctx->Array.TexCoord[texUnit].Ptr = (void *) ptr;

   ctx->Array.TexCoordFunc[texUnit] = gl_trans_4f_tab[size][TYPE_IDX(type)];
   ctx->Array.TexCoordEltFunc[texUnit] = gl_trans_elt_4f_tab[size][TYPE_IDX(type)];
   ctx->Array.NewArrayState |= PIPE_TEX(texUnit);
   ctx->NewState |= NEW_CLIENT_STATE;
}




void GLAPIENTRY glEdgeFlagPointer(CTX_ARG GLsizei stride, const void *vptr )
{
   const GLboolean *ptr = (GLboolean *)vptr;
   
   GLcontext *ctx;
   GET_CONTEXT;
   CHECK_CONTEXT;
   ctx = CC;

   if (stride<0) {
      gl_error( ctx, GL_INVALID_VALUE, "glEdgeFlagPointer(stride)" );
      return;
   }
   ctx->Array.EdgeFlag.Stride = stride;
   ctx->Array.EdgeFlag.StrideB = stride ? stride : sizeof(GLboolean);
   ctx->Array.EdgeFlag.Ptr = (GLboolean *) ptr;
   if (stride != sizeof(GLboolean)) {
      ctx->Array.EdgeFlagFunc = gl_trans_1ub_tab[TYPE_IDX(GL_UNSIGNED_BYTE)];
   } else {
      ctx->Array.EdgeFlagFunc = 0;
   }
   ctx->Array.EdgeFlagEltFunc = gl_trans_elt_1ub_tab[TYPE_IDX(GL_UNSIGNED_BYTE)];
   ctx->Array.NewArrayState |= VERT_EDGE;
   ctx->NewState |= NEW_CLIENT_STATE;
}


/* Called only from gl_DrawElements
 */
void gl_CVAEltPointer( GLcontext *ctx, GLenum type, const GLvoid *ptr )
{
   switch (type) {
      case GL_UNSIGNED_BYTE:
         ctx->CVA.Elt.StrideB = sizeof(GLubyte);
         break;
      case GL_UNSIGNED_SHORT:
         ctx->CVA.Elt.StrideB = sizeof(GLushort);
         break;
      case GL_UNSIGNED_INT:
         ctx->CVA.Elt.StrideB = sizeof(GLuint);
         break;
      default:
         gl_error( ctx, GL_INVALID_ENUM, "glEltPointer(type)" );
         return;
   }
   ctx->CVA.Elt.Type = type;
   ctx->CVA.Elt.Stride = 0;
   ctx->CVA.Elt.Ptr = (void *) ptr;
   ctx->CVA.EltFunc = gl_trans_1ui_tab[TYPE_IDX(type)];
   ctx->Array.NewArrayState |= VERT_ELT; /* ??? */
}



/* KW: Batch function to exec all the array elements in the input
 *     buffer prior to transform.  Done only the first time a vertex
 *     buffer is executed or compiled.
 */
void gl_exec_array_elements( GLcontext *ctx, struct immediate *IM )
{
   GLuint *flags = IM->Flag;
   GLuint *elts = IM->Elt;
   GLuint count = IM->Count;
   GLuint start = IM->Start;
   GLuint translate = ctx->Array.Flags;
   GLuint i;
   
   if (translate & VERT_OBJ_ANY) 
      (ctx->Array.VertexEltFunc)( IM->Obj, 
				  &ctx->Array.Vertex, 
				  flags, elts, VERT_ELT,
				  start, count);
   
   if (translate & VERT_NORM) 
      (ctx->Array.NormalEltFunc)( IM->Normal, 
				  &ctx->Array.Normal, 
				  flags, elts, (VERT_ELT|VERT_NORM),
				  start, count);

   if (translate & VERT_EDGE) 
      (ctx->Array.EdgeFlagEltFunc)( IM->EdgeFlag, 
				    &ctx->Array.EdgeFlag, 
				    flags, elts, (VERT_ELT|VERT_EDGE),
				    start, count);
   
   if (translate & VERT_RGBA)
      (ctx->Array.ColorEltFunc)( IM->Color, 
				 &ctx->Array.Color, 
				 flags, elts, (VERT_ELT|VERT_RGBA),
				 start, count);

   if (translate & VERT_INDEX)
      (ctx->Array.IndexEltFunc)( IM->Index, 
				 &ctx->Array.Index, 
				 flags, elts, (VERT_ELT|VERT_INDEX),
				 start, count);

   if (translate & VERT_TEX0_ANY)
      (ctx->Array.TexCoordEltFunc[0])( IM->TexCoord[0], 
				       &ctx->Array.TexCoord[0], 
				       flags, elts, (VERT_ELT|VERT_TEX0_ANY),
				       start, count);

   if (translate & VERT_TEX1_ANY)
      (ctx->Array.TexCoordEltFunc[1])( IM->TexCoord[1], 
				       &ctx->Array.TexCoord[1], 
				       flags, elts, (VERT_ELT|VERT_TEX1_ANY),
				       start, count);

   IM->OrFlag |= translate;

   /* Lighting ignores the and-flag, so still need to do this.
    */
   if (IM->AndFlag & VERT_ELT) {
      for (i = 0 ; i < count ; i++) 
	 flags[i] |= translate;
      IM->AndFlag |= translate; 
   } else {
      GLuint andflag = ~0;
      for (i = 0 ; i < count ; i++) {
	 if (flags[i] & VERT_ELT) flags[i] |= translate;
	 andflag &= flags[i];
      }
      IM->AndFlag = andflag;
   }
}



/* KW:  I think this is moving in the right direction, but it still feels 
 *      like we are doing way too much work.
 */
void gl_DrawArrays( GLcontext *ctx, GLenum mode, GLint start, GLsizei count )
{
   struct vertex_buffer *VB = ctx->VB;
   GLint i;

   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glDrawArrays");

   if (count<0) {
      gl_error( ctx, GL_INVALID_VALUE, "glDrawArrays(count)" );
      return;
   }

   if (!ctx->CompileFlag && ctx->Array.Vertex.Enabled)
   {
      GLint remaining = count;
      GLint i;
      GLvector4f obj;
      GLvector3f norm;
      GLvector4f tc[MAX_TEXTURE_UNITS];
      GLvector4ub col;
      GLvector1ub edge;
      GLvector1ui index;
      GLuint update = 0, translate = 0;
      struct vertex_array_pointers VSrc;
      struct immediate *IM = VB->IM;
      struct gl_client_array *client_data;
      struct gl_pipeline *elt = &ctx->CVA.elt;
      GLuint relock;
      GLuint fallback, required;

      if (ctx->NewState)
	 gl_update_state( ctx );	

      /* This will die miserably with CVA...  Need more work to support this.
       */
      relock = ctx->CompileCVAFlag;
      ctx->CompileCVAFlag = 0;

      if (!elt->pipeline_valid || relock)
	 gl_build_immediate_pipeline( ctx );

      required = elt->inputs;
      fallback = (elt->inputs & ~ctx->Array.Summary);

      VSrc.Color = &IM->v.Color;
      VSrc.Index = &IM->v.Index;
      VSrc.EdgeFlag = &IM->v.EdgeFlag;
      VSrc.TexCoord[0] = &IM->v.TexCoord[0];
      VSrc.TexCoord[1] = &IM->v.TexCoord[1];
      VSrc.Obj = &IM->v.Obj;
      VSrc.Normal = &IM->v.Normal;

      if (required & VERT_RGBA) 
      {
	 client_data = &ctx->Array.Color;
	 if (fallback & VERT_RGBA)
	    client_data = &ctx->Fallback.Color;

	 if (client_data->Type == GL_UNSIGNED_BYTE && 
	     client_data->Size == 4)
	 {
	    VSrc.Color = &col;
	    col.data = (GLubyte (*)[4]) client_data->Ptr;
	    col.stride = client_data->StrideB;
	    col.flags = VEC_NOT_WRITABLE|VEC_GOOD_STRIDE;
	    if (client_data->StrideB != 4 * sizeof(GLubyte)) 
	       col.flags ^= VEC_STRIDE_FLAGS;

	    update |= VERT_RGBA;
	 } else {
	    translate |= VERT_RGBA;
	 }	
      }
   
      if (required & VERT_INDEX) 
      {
	 client_data = &ctx->Array.Index;
	 if (fallback & VERT_INDEX)
	    client_data = &ctx->Fallback.Index;

	 if (client_data->Type == GL_UNSIGNED_INT)
	 {
	    VSrc.Index = &index;
	    index.data = (GLuint *) client_data->Ptr;
	    index.stride = client_data->StrideB;
	    index.flags = VEC_NOT_WRITABLE|VEC_GOOD_STRIDE;
	    if (client_data->StrideB != sizeof(GLuint)) 
	       index.flags ^= VEC_STRIDE_FLAGS;

	    update |= VERT_INDEX;
	 } else {
	    translate |= VERT_INDEX;
	 }	
      }

      for (i = 0 ; i < MAX_TEXTURE_UNITS ; i++) 
      {
	 GLuint flag = VERT_TEX_ANY(i);

	 if (required & flag) {

	    client_data = &ctx->Array.TexCoord[i];

	    if (fallback & flag) 
	    {
	       client_data = &ctx->Fallback.TexCoord[i];
	       client_data->Size = gl_texcoord_size( ctx->Current.Flag, i );
	    }

	    if (client_data->Type == GL_FLOAT)
	    {
	       VSrc.TexCoord[i] = &tc[i];
	       tc[i].data = (GLfloat (*)[4]) client_data->Ptr;
	       tc[i].stride = client_data->StrideB;
	       tc[i].size = client_data->Size;
	       tc[i].flags = VEC_NOT_WRITABLE|VEC_GOOD_STRIDE;
	       if (tc[i].stride |= 4 * sizeof(GLfloat))
		  tc[i].flags ^= VEC_STRIDE_FLAGS;
	       update |= flag;
	    } else {
	       translate |= flag;
	    }
	 }
      }

      if (ctx->Array.Flags != ctx->Array.Flag[0])
	 for (i = 0 ; i < VB_MAX ; i++) 
	    ctx->Array.Flag[i] = ctx->Array.Flags;

      
      if (ctx->Array.Vertex.Type == GL_FLOAT)
      {
	 VSrc.Obj = &obj;
	 obj.data = (GLfloat (*)[4]) ctx->Array.Vertex.Ptr;
	 obj.stride = ctx->Array.Vertex.StrideB;
	 obj.size = ctx->Array.Vertex.Size;
	 obj.flags = VEC_NOT_WRITABLE|VEC_GOOD_STRIDE;
	 if (obj.stride != 4 * sizeof(GLfloat)) 
	    obj.flags ^= VEC_STRIDE_FLAGS;

	 update |= VERT_OBJ_ANY;
      }
      else 
      {
	 translate |= VERT_OBJ_ANY;
      }

      if (required & VERT_NORM) 
      {
	 client_data = &ctx->Array.Normal;
	 if (fallback & VERT_NORM) 
	    client_data = &ctx->Fallback.Normal;

	 if (client_data->Type == GL_FLOAT) 
	 {
	    VSrc.Normal = &norm;
	    norm.flags = 0;
	    norm.data = (GLfloat (*)[3]) client_data->Ptr;
	    norm.stride = client_data->StrideB;
	    update |= VERT_NORM;	    
	 } else {
	    translate |= VERT_NORM;	    
	 }
      }

      if ( (required & VERT_EDGE) && 
	   (mode == GL_TRIANGLES || 
	    mode == GL_QUADS || 
	    mode == GL_POLYGON)) 
      {
	 client_data = &ctx->Array.EdgeFlag;

	 if (fallback & VERT_EDGE) 
	    client_data = &ctx->Fallback.EdgeFlag;

	 VSrc.EdgeFlag = &edge;
	 edge.data = (GLboolean *) client_data->Ptr;
	 edge.stride = client_data->StrideB;
	 edge.flags = VEC_NOT_WRITABLE|VEC_GOOD_STRIDE;
	 if (edge.stride != sizeof(GLubyte))
	    edge.flags ^= VEC_STRIDE_FLAGS;
	 
	 update |= VERT_EDGE;
      }

      VB->Primitive = IM->Primitive; 
      VB->NextPrimitive = IM->NextPrimitive; 
      VB->MaterialMask = IM->MaterialMask;
      VB->Material = IM->Material;
      VB->BoundsPtr = 0;

      while (remaining > 0) {
         GLint vbspace = VB_MAX - VB_START;
	 GLuint count, n;
	 

	 if (vbspace >= remaining) {
	    n = remaining;
	    VB->LastPrimitive = VB_START + n;
	 } else {
	    n = vbspace;
	    VB->LastPrimitive = VB_START;
	 }
	 
	 VB->CullMode = 0;
	 

         /* Update pointers.
	  */
	 if (update) {
	    if (update & VERT_OBJ_ANY) 
	       obj.start = VEC_ELT(&obj, GLfloat, start);
	    
	    if (update & VERT_NORM) 
	       norm.start = VEC_ELT(&norm, GLfloat, start);

	    if (update & VERT_EDGE) 
	       edge.start = VEC_ELT(&edge, GLubyte, start);

	    if (update & VERT_RGBA) 
	       col.start = VEC_ELT(&col, GLubyte, start);

	    if (update & VERT_INDEX) 
	       index.start = VEC_ELT(&index, GLuint, start);

	    if (update & VERT_TEX0_ANY) 
	       tc[0].start = VEC_ELT(&tc[0], GLfloat, start);

	    if (update & VERT_TEX1_ANY) 
	       tc[1].start = VEC_ELT(&tc[1], GLfloat, start);
	 }


	 /* Translate data to fix up type and stride.
	  */
	 if (translate) {
	    if (translate & VERT_OBJ_ANY) {
	       ctx->Array.VertexFunc( IM->Obj + VB_START, 
				      &ctx->Array.Vertex, start, n );
	    }

	    if (translate & VERT_NORM) {
	       ctx->Array.NormalFunc( IM->Normal + VB_START, 
				      &ctx->Array.Normal, start, n );
	    }

	    if (translate & VERT_EDGE) {
	       ctx->Array.EdgeFlagFunc( IM->EdgeFlag + VB_START, 
					&ctx->Array.EdgeFlag, start, n );
	    }

	    if (translate & VERT_RGBA) {
	       ctx->Array.ColorFunc( IM->Color + VB_START, 
				     &ctx->Array.Color, start, n );
	    }

	    if (translate & VERT_INDEX) {
	       ctx->Array.IndexFunc( IM->Index + VB_START, 
				     &ctx->Array.Index, start, n );
	    }

	    if (translate & VERT_TEX0_ANY) {
	       IM->v.TexCoord[0].size = tc[0].size;
	       ctx->Array.TexCoordFunc[0]( IM->TexCoord[0] + VB_START, 
					   &ctx->Array.TexCoord[0], start, n );
	    }

	    if (translate & VERT_TEX1_ANY) {
	       IM->v.TexCoord[1].size = tc[1].size;
	       ctx->Array.TexCoordFunc[1]( IM->TexCoord[1] + VB_START, 
					   &ctx->Array.TexCoord[1], start, n );
	    }
	 }


	 VB->ObjPtr = VSrc.Obj;
	 VB->NormalPtr = VSrc.Normal;
	 VB->Color[0] = VB->Color[1] = VB->ColorPtr = VSrc.Color;
	 VB->IndexPtr = VSrc.Index;
	 VB->EdgeFlagPtr = VSrc.EdgeFlag;
	 VB->TexCoordPtr[0] = VSrc.TexCoord[0];
	 VB->TexCoordPtr[1] = VSrc.TexCoord[1];

	 VB->Flag = ctx->Array.Flag;
	 VB->AndFlag = ctx->Array.Flags;
	 VB->OrFlag = ctx->Array.Flags;

	 count = VB->Count = VB_START + n;

	 VB->ObjPtr->count = count;
	 VB->NormalPtr->count = count;
	 VB->ColorPtr->count = count;
	 VB->IndexPtr->count = count;
	 VB->EdgeFlagPtr->count = count;
	 VB->TexCoordPtr[0]->count = count;
	 VB->TexCoordPtr[1]->count = count;

	 VB->Flag[count] |= VERT_END_VB;
	 VB->Flag[VB_START] |= VERT_NORM;
/*  	 VB->Flag[VB_START] |= (IM->Flag[vb_start] & VERT_MATERIAL); */

	 VB->NextPrimitive[VB->CopyStart] = VB->Count;
	 VB->Primitive[VB->CopyStart] = mode;

         /* Transform and render.
	  */
         gl_run_pipeline( VB );
	 gl_reset_vb( VB );

	 ctx->Array.Flag[count] = ctx->Array.Flags;
	 ctx->Array.Flag[VB_START] = ctx->Array.Flags;
	 IM->Flag[VB_START] = 0;

         start += n;
         remaining -= n;
      }

      ctx->CompileCVAFlag = relock;
   }
   else if (ctx->Array.Vertex.Enabled) 
   {
      /* The GL_COMPILE and GL_COMPILE_AND_EXECUTE cases.  These
       * could be handled by the above code, but it gets a little
       * complex.  
       */
      /* No need to reset - never called from inside a display list */
      gl_Begin( ctx, mode );
      for (i=0;i<count;i++) {
         gl_ArrayElement( ctx, start+i );
      }
      gl_End( ctx );
   }
   else
   {
      /* The degenerate case where vertices are not enabled - only
       * need to process the very final array element, as all of the
       * preceding ones would be overwritten anyway. 
       */
      gl_Begin( ctx, mode );
      gl_ArrayElement( ctx, start+count );
      gl_End( ctx );
   }
}



/* KW: Exactly fakes the effects of calling glArrayElement multiple times.
 *     Compilation is handled via. the IM->maybe_transform_vb() callback.
 */
#define DRAW_ELT(FUNC, TYPE)				\
static void FUNC( GLcontext *ctx, GLenum mode,		\
		  TYPE *indices, GLuint count )		\
{							\
   GLuint i,j;						\
							\
   if (count) gl_Begin( ctx, mode );			\
							\
   for (j = 0 ; j < count ; ) {				\
      GLuint nr = MIN2( VB_MAX, count - j + VB_START );	\
      struct immediate *IM = ctx->input;		\
      GLuint sf = IM->Flag[VB_START];			\
      GLuint flags = IM->ArrayOrFlags;			\
							\
      for (i = VB_START ; i < nr ; i++) {		\
	 IM->Elt[i] = (GLuint) *indices++;		\
	 IM->Flag[i] = flags;				\
      }							\
							\
      if (j == 0) IM->Flag[VB_START] |= sf;		\
							\
      IM->Count = nr;					\
      j += nr - VB_START;				\
							\
      if (j == count) gl_End( ctx );			\
							\
      IM->maybe_transform_vb( IM );			\
   }							\
}

DRAW_ELT( draw_elt_ubyte, GLubyte )
DRAW_ELT( draw_elt_ushort, GLushort )
DRAW_ELT( draw_elt_uint, GLuint )


static GLuint natural_stride[0x10] = 
{
   sizeof(GLbyte),		/* 0 */
   sizeof(GLubyte),		/* 1 */
   sizeof(GLshort),		/* 2 */
   sizeof(GLushort),		/* 3 */
   sizeof(GLint),		/* 4 */
   sizeof(GLuint),		/* 5 */
   sizeof(GLfloat),		/* 6 */
   2 * sizeof(GLbyte),		/* 7 */
   3 * sizeof(GLbyte),		/* 8 */
   4 * sizeof(GLbyte),		/* 9 */
   sizeof(GLdouble),		/* a */
   0,				/* b */
   0,				/* c */
   0,				/* d */
   0,				/* e */
   0				/* f */
};

void GLAPIENTRY glDrawElements(CTX_ARG GLenum mode, GLsizei count,
                                GLenum type, const GLvoid *indices )
{
   GLcontext *ctx;
   struct gl_cva *cva;
      
   GET_CONTEXT;
   CHECK_CONTEXT;
   ctx = CC;

   cva = &ctx->CVA;
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx, "glDrawElements");

   if (count<0) {
      gl_error( ctx, GL_INVALID_VALUE, "glDrawElements(count)" );
      return;
   }

   if (mode < 0 || mode > GL_POLYGON) {
      gl_error( ctx, GL_INVALID_ENUM, "glDrawArrays(mode)" );
      return;
   }
   
   if (type != GL_UNSIGNED_INT && type != GL_UNSIGNED_BYTE && type != GL_UNSIGNED_SHORT)
   {
       gl_error( ctx, GL_INVALID_ENUM, "glDrawElements(type)" );
       return;
   }

   if (ctx->NewState)
      gl_update_state(ctx);

   if (ctx->CompileCVAFlag) 
   {
#if defined(MESA_CVA_PROF)
      force_init_prof(); 
#endif

      /* Treat VERT_ELT like a special client array.
       */
      ctx->Array.NewArrayState |= VERT_ELT;
      ctx->Array.Summary |= VERT_ELT;
      ctx->Array.Flags |= VERT_ELT;

      cva->elt_mode = mode;
      cva->elt_count = count;
      cva->Elt.Type = type;
      cva->Elt.Ptr = (void *) indices;
      cva->Elt.StrideB = natural_stride[TYPE_IDX(type)];
      cva->EltFunc = gl_trans_1ui_tab[TYPE_IDX(type)];

      if (!cva->pre.pipeline_valid) 
	 gl_build_precalc_pipeline( ctx );
      else if (MESA_VERBOSE & VERBOSE_PIPELINE)
	 fprintf(stderr, ": dont rebuild\n");

      gl_cva_force_precalc( ctx );

      /* Did we 'precalculate' the render op?
       */
      if (ctx->CVA.pre.ops & PIPE_OP_RENDER) {
	 ctx->Array.NewArrayState |= VERT_ELT;
	 ctx->Array.Summary &= ~VERT_ELT;
	 ctx->Array.Flags &= ~VERT_ELT;
	 return;
      } 

      if ( (MESA_VERBOSE&VERBOSE_VARRAY) )
	 printf("using immediate\n");
   }


   /* Otherwise, have to use the immediate path to render.
    */
   switch (type) {
   case GL_UNSIGNED_BYTE:
   {
      GLubyte *ub_indices = (GLubyte *) indices;
      if (ctx->Array.Summary & VERT_OBJ_ANY) {
	 draw_elt_ubyte( ctx, mode, ub_indices, count );
      } else {
	 gl_ArrayElement( ctx, (GLuint) ub_indices[count-1] );
      }
   }
   break;
   case GL_UNSIGNED_SHORT:
   {
      GLushort *us_indices = (GLushort *) indices;
      if (ctx->Array.Summary & VERT_OBJ_ANY) {
	 draw_elt_ushort( ctx, mode, us_indices, count );
      } else {
	 gl_ArrayElement( ctx, (GLuint) us_indices[count-1] );
      }
   }
   break;
   case GL_UNSIGNED_INT:
   {
      GLuint *ui_indices = (GLuint *) indices;
      if (ctx->Array.Summary & VERT_OBJ_ANY) {
	 draw_elt_uint( ctx, mode, ui_indices, count );
      } else {
	 gl_ArrayElement( ctx, ui_indices[count-1] );
      }
   }
   break;
   default:
      gl_error( ctx, GL_INVALID_ENUM, "glDrawElements(type)" );
      break;
   }

   if (ctx->CompileCVAFlag) {
      ctx->Array.NewArrayState |= VERT_ELT;
      ctx->Array.Summary &= ~VERT_ELT;
   }
}



void GLAPIENTRY glInterleavedArrays(CTX_ARG GLenum format, GLsizei stride,
                                     const GLvoid *pointer )
{
   GLcontext *ctx;
   GLboolean tflag, cflag, nflag;  /* enable/disable flags */
   GLint tcomps, ccomps, vcomps;   /* components per texcoord, color, vertex */

   GLenum ctype;                   /* color type */
   GLint coffset, noffset, voffset;/* color, normal, vertex offsets */
   GLint defstride;                /* default stride */
   GLint c, f;
   GLint coordUnitSave;
   
   GET_CONTEXT;
   CHECK_CONTEXT;
   ctx = CC;


   f = sizeof(GLfloat);
   c = f * ((4*sizeof(GLubyte) + (f-1)) / f);

   if (stride<0) {
      gl_error( ctx, GL_INVALID_VALUE, "glInterleavedArrays(stride)" );
      return;
   }

   switch (format) {
      case GL_V2F:
         tflag = GL_FALSE;  cflag = GL_FALSE;  nflag = GL_FALSE;
         tcomps = 0;  ccomps = 0;  vcomps = 2;
         voffset = 0;
         defstride = 2*f;
         break;
      case GL_V3F:
         tflag = GL_FALSE;  cflag = GL_FALSE;  nflag = GL_FALSE;
         tcomps = 0;  ccomps = 0;  vcomps = 3;
         voffset = 0;
         defstride = 3*f;
         break;
      case GL_C4UB_V2F:
         tflag = GL_FALSE;  cflag = GL_TRUE;  nflag = GL_FALSE;
         tcomps = 0;  ccomps = 4;  vcomps = 2;
         ctype = GL_UNSIGNED_BYTE;
         coffset = 0;
         voffset = c;
         defstride = c + 2*f;
         break;
      case GL_C4UB_V3F:
         tflag = GL_FALSE;  cflag = GL_TRUE;  nflag = GL_FALSE;
         tcomps = 0;  ccomps = 4;  vcomps = 3;
         ctype = GL_UNSIGNED_BYTE;
         coffset = 0;
         voffset = c;
         defstride = c + 3*f;
         break;
      case GL_C3F_V3F:
         tflag = GL_FALSE;  cflag = GL_TRUE;  nflag = GL_FALSE;
         tcomps = 0;  ccomps = 3;  vcomps = 3;
         ctype = GL_FLOAT;
         coffset = 0;
         voffset = 3*f;
         defstride = 6*f;
         break;
      case GL_N3F_V3F:
         tflag = GL_FALSE;  cflag = GL_FALSE;  nflag = GL_TRUE;
         tcomps = 0;  ccomps = 0;  vcomps = 3;
         noffset = 0;
         voffset = 3*f;
         defstride = 6*f;
         break;
      case GL_C4F_N3F_V3F:
         tflag = GL_FALSE;  cflag = GL_TRUE;  nflag = GL_TRUE;
         tcomps = 0;  ccomps = 4;  vcomps = 3;
         ctype = GL_FLOAT;
         coffset = 0;
         noffset = 4*f;
         voffset = 7*f;
         defstride = 10*f;
         break;
      case GL_T2F_V3F:
         tflag = GL_TRUE;  cflag = GL_FALSE;  nflag = GL_FALSE;
         tcomps = 2;  ccomps = 0;  vcomps = 3;
         voffset = 2*f;
         defstride = 5*f;
         break;
      case GL_T4F_V4F:
         tflag = GL_TRUE;  cflag = GL_FALSE;  nflag = GL_FALSE;
         tcomps = 4;  ccomps = 0;  vcomps = 4;
         voffset = 4*f;
         defstride = 8*f;
         break;
      case GL_T2F_C4UB_V3F:
         tflag = GL_TRUE;  cflag = GL_TRUE;  nflag = GL_FALSE;
         tcomps = 2;  ccomps = 4;  vcomps = 3;
         ctype = GL_UNSIGNED_BYTE;
         coffset = 2*f;
         voffset = c+2*f;
         defstride = c+5*f;
         break;
      case GL_T2F_C3F_V3F:
         tflag = GL_TRUE;  cflag = GL_TRUE;  nflag = GL_FALSE;
         tcomps = 2;  ccomps = 3;  vcomps = 3;
         ctype = GL_FLOAT;
         coffset = 2*f;
         voffset = 5*f;
         defstride = 8*f;
         break;
      case GL_T2F_N3F_V3F:
         tflag = GL_TRUE;  cflag = GL_FALSE;  nflag = GL_TRUE;
         tcomps = 2;  ccomps = 0;  vcomps = 3;
         noffset = 2*f;
         voffset = 5*f;
         defstride = 8*f;
         break;
      case GL_T2F_C4F_N3F_V3F:
         tflag = GL_TRUE;  cflag = GL_TRUE;  nflag = GL_TRUE;
         tcomps = 2;  ccomps = 4;  vcomps = 3;
         ctype = GL_FLOAT;
         coffset = 2*f;
         noffset = 6*f;
         voffset = 9*f;
         defstride = 12*f;
         break;
      case GL_T4F_C4F_N3F_V4F:
         tflag = GL_TRUE;  cflag = GL_TRUE;  nflag = GL_TRUE;
         tcomps = 4;  ccomps = 4;  vcomps = 4;
         ctype = GL_FLOAT;
         coffset = 4*f;
         noffset = 8*f;
         voffset = 11*f;
         defstride = 15*f;
         break;
      default:
         gl_error( ctx, GL_INVALID_ENUM, "glInterleavedArrays(format)" );
         return;
   }

   if (stride==0) {
      stride = defstride;
   }

   gl_DisableClientState( ctx, GL_EDGE_FLAG_ARRAY );
   gl_DisableClientState( ctx, GL_INDEX_ARRAY );

   /* Texcoords */
   coordUnitSave = ctx->TexCoordUnit;
   if (tflag) {
      GLint i;
      GLint factor = ctx->Array.TexCoordInterleaveFactor;
      for (i = 0; i < factor; i++) {
         gl_ActiveTexture( ctx, (GLenum) (GL_TEXTURE0_ARB + i) );
         gl_EnableClientState( ctx, GL_TEXTURE_COORD_ARRAY );
         glTexCoordPointer(CTX_PRM  tcomps, GL_FLOAT, stride,
                             (GLubyte *) pointer + i * coffset );
      }
      for (i = factor; i < ctx->Const.MaxTextureUnits; i++) {
         gl_ActiveTexture( ctx, (GLenum) (GL_TEXTURE0_ARB + i) );
         gl_DisableClientState( ctx, GL_TEXTURE_COORD_ARRAY );
      }
   }
   else {
      GLint i;
      for (i = 0; i < ctx->Const.MaxTextureUnits; i++) {
         gl_ActiveTexture( ctx, (GLenum) (GL_TEXTURE0_ARB + i) );
         gl_DisableClientState( ctx, GL_TEXTURE_COORD_ARRAY );
      }
   }
   /* Restore texture coordinate unit index */
   gl_ActiveTexture( ctx, (GLenum) (GL_TEXTURE0_ARB + coordUnitSave) );


   /* Color */
   if (cflag) {
      gl_EnableClientState( ctx, GL_COLOR_ARRAY );
      glColorPointer(CTX_PRM ccomps, ctype, stride,
                       (GLubyte*) pointer + coffset );
   }
   else {
      gl_DisableClientState( ctx, GL_COLOR_ARRAY );
   }


   /* Normals */
   if (nflag) {
      gl_EnableClientState( ctx, GL_NORMAL_ARRAY );
      glNormalPointer(CTX_PRM GL_FLOAT, stride,
                        (GLubyte*) pointer + noffset );
   }
   else {
      gl_DisableClientState( ctx, GL_NORMAL_ARRAY );
   }

   gl_EnableClientState( ctx, GL_VERTEX_ARRAY );
   glVertexPointer(CTX_PRM vcomps, GL_FLOAT, stride,
                     (GLubyte *) pointer + voffset );
}



void GLAPIENTRY glDrawRangeElements(CTX_ARG GLenum mode, GLuint start,
                                     GLuint end, GLsizei count,
                                     GLenum type, const GLvoid *indices )
{
   GLcontext *ctx;
   GET_CONTEXT;
   CHECK_CONTEXT;
   ctx = CC;

   if (end < start) {
      gl_error(ctx, GL_INVALID_VALUE, "glDrawRangeElements( end < start )");
      return;
   }

   if (!ctx->Array.LockCount && 2*count > 3*(end-start)) {
      glLockArraysEXT(CTX_PRM start, end );
      glDrawElements(CTX_PRM mode, count, type, indices );
      glUnlockArraysEXT(CTX_VPRM );
   } else {
      glDrawElements(CTX_PRM mode, count, type, indices );
   }
}



void gl_update_client_state( GLcontext *ctx )
{
   static GLuint sz_flags[5] = { 0, 
				 0,
				 VERT_OBJ_2, 
				 VERT_OBJ_23, 
				 VERT_OBJ_234 };

   static GLuint tc_flags[5] = { 0, 
				 VERT_TEX0_1,
				 VERT_TEX0_12, 
				 VERT_TEX0_123, 
				 VERT_TEX0_1234 };

   ctx->Array.Flags = 0;
   ctx->Array.Summary = 0;
   ctx->input->ArrayIncr = 0;
   
   if (ctx->Array.Normal.Enabled)      ctx->Array.Flags |= VERT_NORM;
   if (ctx->Array.Color.Enabled)       ctx->Array.Flags |= VERT_RGBA;
   if (ctx->Array.Index.Enabled)       ctx->Array.Flags |= VERT_INDEX;
   if (ctx->Array.EdgeFlag.Enabled)    ctx->Array.Flags |= VERT_EDGE;
   if (ctx->Array.Vertex.Enabled) {
      ctx->Array.Flags |= sz_flags[ctx->Array.Vertex.Size];
      ctx->input->ArrayIncr = 1;
   }
   if (ctx->Array.TexCoord[0].Enabled) {
      ctx->Array.Flags |= tc_flags[ctx->Array.TexCoord[0].Size];
   }
   if (ctx->Array.TexCoord[1].Enabled) {
      ctx->Array.Flags |= (tc_flags[ctx->Array.TexCoord[1].Size] << NR_TEXSIZE_BITS);
   }

   /* Not really important any more:
    */
   ctx->Array.Summary = ctx->Array.Flags & VERT_DATA;

   ctx->input->ArrayOrFlags = (ctx->Array.Flags & VERT_OBJ_234) | VERT_ELT;
   ctx->input->ArrayAndFlags = ~ctx->Array.Flags;
   ctx->input->ArrayEltFlush = !(ctx->CompileCVAFlag);
}

