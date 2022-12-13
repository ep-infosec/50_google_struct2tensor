/* Copyright 2019 Google LLC

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/shape_inference.h"

using tensorflow::shape_inference::InferenceContext;

// Represents each field as two vectors (data and index) of equal length.
// Each element of data contains the value of an element in the field.
// The corresponding element of index is the index of the protocol buffer
// that the element came from. Values are found in the order that they
// occur in the protocol buffer.
REGISTER_OP("DecodeProtoSparseV4")
    .Input("bytes: string")
    .Input("backing_string: num_backing_string * string")
    .Attr("num_backing_string: int >= 0 = 0")
    .Attr("message_type: string")
    .Attr("field_names: list(string)")
    .Attr("num_fields: int")
    .Attr("output_types: list(type) >= 0")
    .Attr("descriptor_literal: string = ''")
    .Attr("descriptor_source: string = 'local://'")
    .Attr("message_format: string = 'binary'")
    .Attr("sanitize: bool = false")
    .Attr("honor_proto3_optional_semantics: bool = false")
    .Output("values: output_types")
    .Output("indices: num_fields * int64")
    .SetShapeFn([](InferenceContext* c) {
      std::vector<tensorflow::DataType> output_types;
      TF_RETURN_IF_ERROR(c->GetAttr("output_types", &output_types));

      // TODO(martinz): for required fields, we would know the shape.
      for (int i = 0; i < 2 * output_types.size(); ++i) {
        c->set_output(i, c->Vector(c->UnknownDim()));
      }

      return tensorflow::OkStatus();
    })
    .Doc(R"doc(
The `decode_proto_sparse` op extracts fields from a serialized protocol
buffers message into TensorFlow Tensors.  The fields in `field_names`
are decoded and converted to the corresponding `output_types` if
possible.

A `message_type` name must be provided to give context for the field
names. The actual message descriptor can be decoded from the binary
serialization of a file_descriptor_set proto in descriptor_literal, or it can
be looked up either in the linked-in descriptor pool, the global protocol
buffer database, or a filename provided by the caller using the
`descriptor_source` attribute.

Represents each field as two vectors (data and index) of equal length.
Each element of data contains the value of an element in the field.
The corresponding element of index is the index of the protocol buffer
that the element came from.
values=concat(map(lambda x:x.foo()))
Values are found in the order that they
occur in the protocol buffer.

For the most part, the mapping between Proto field types and
TensorFlow dtypes is straightforward. However, there are a few
special cases:

- A proto field that contains a submessage or group can only be converted
to `DT_STRING` (the serialized submessage). This is to reduce the
complexity of the API. The resulting string can be used as input
to another instance of the decode_proto op.

- TensorFlow lacks support for unsigned integers. The ops represent uint64
types as a `DT_INT64` with the same twos-complement bit pattern
(the obvious way). Unsigned int32 values can be represented exactly by
specifying type `DT_INT64`, or using twos-complement if the caller
specifies `DT_INT32` in the `output_types` attribute.

The `descriptor_source` attribute selects a source of protocol
descriptors to consult when looking up `message_type`. This may be a
filename containing a serialized `proto2.FileDescriptorSet` message,
or one of the two special values `local://` and `global://`.
- `local://`: only descriptors linked into the code will be searched
- `global://`: the global protocol database will be used to look up descriptors
The default is `local://`. The filename can be on any filesystem accessible to
TensorFlow.


The `local://` database only covers descriptors linked into the
code via C++ libraries, not Python imports. You can link in a proto descriptor
by creating a cc_library target with alwayslink=1.

Both binary and text proto serializations are supported, and can be
chosen using the `format` attribute.

If `honor_proto3_optional_semantics` is true, if a proto3 primitive optional
field without the presence semantic (i.e. the field is without the "optional" or
"repeated" label) is requested to be parsed, it will always have a value for
each input parent message. If a value is not present on wire, the default value
(0 or "") will be used.

bytes: tensor of serialized protos with shape `batch_shape`.
backing_string: a list of string tensors which backs the input `bytes`
  for using string_views. This is an optimization to prevent alloc/dealloc of
  subtree serialized protos tensors. This input is not functionally used other
  than to keep the backing string alive in memory. If set, serialized
  sub-messages decoded by this op will be string_views pointing to
  the input `bytes` (which might also be a string_view).
num_backing_string: The number of backing_string inputs. Default to 0 and can be
  empty to allow backward compatility.
message_type: name of the proto message type to decode.
field_names: list of strings containing proto field names.
num_fields: len(field_names)
output_types: list of TF types to use for the respective field in field_names.
TODO(b/69051300): Consider extending options in descriptor_source.
descriptor_source: one of `local://', `global://`, or the path to file
   containing a serialized `proto2.FileDescriptorSet`.
message_format: either `binary` or `text`.
values: list of tensors containing values for the corresponding field.
   `values[i]` has datatype `output_types[i]` and a vector shape.
indices: list of tensors containing values for the corresponding field.
   `indices[i]` is an int64 vector.

)doc");

// See DecodeProtoSparseV4. DecodeProtoSparseV3 does not have attr
// `honor_proto3_optional_semantics`.
REGISTER_OP("DecodeProtoSparseV3")
    .Input("bytes: string")
    .Input("backing_string: num_backing_string * string")
    .Attr("num_backing_string: int >= 0 = 0")
    .Attr("message_type: string")
    .Attr("field_names: list(string)")
    .Attr("num_fields: int")
    .Attr("output_types: list(type) >= 0")
    .Attr("descriptor_literal: string = ''")
    .Attr("descriptor_source: string = 'local://'")
    .Attr("message_format: string = 'binary'")
    .Attr("sanitize: bool = false")
    .Output("values: output_types")
    .Output("indices: num_fields * int64")
    .SetShapeFn([](InferenceContext* c) {
      std::vector<tensorflow::DataType> output_types;
      TF_RETURN_IF_ERROR(c->GetAttr("output_types", &output_types));

      // TODO(martinz): for required fields, we would know the shape.
      for (int i = 0; i < 2 * output_types.size(); ++i) {
        c->set_output(i, c->Vector(c->UnknownDim()));
      }

      return tensorflow::OkStatus();
    });

// See DecodeProtoSparseV3. DecodeProtoSparseV2 omits `backing_string` and
// `num_backing_string` and does not support string_views  for
// intermediate serialized proto outputs.
REGISTER_OP("DecodeProtoSparseV2")
    .Input("bytes: string")
    .Attr("message_type: string")
    .Attr("field_names: list(string)")
    .Attr("num_fields: int")
    .Attr("output_types: list(type) >= 0")
    .Attr("descriptor_literal: string = ''")
    .Attr("descriptor_source: string = 'local://'")
    .Attr("message_format: string = 'binary'")
    .Attr("sanitize: bool = false")
    .Output("values: output_types")
    .Output("indices: num_fields * int64")
    .SetShapeFn([](InferenceContext* c) {
      std::vector<tensorflow::DataType> output_types;
      TF_RETURN_IF_ERROR(c->GetAttr("output_types", &output_types));

      // TODO(martinz): for required fields, we would know the shape.
      for (int i = 0; i < 2 * output_types.size(); ++i) {
        c->set_output(i, c->Vector(c->UnknownDim()));
      }

      return tensorflow::OkStatus();
    });
