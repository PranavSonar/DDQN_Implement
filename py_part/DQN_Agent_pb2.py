# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: DQN_Agent.proto

import sys
_b=sys.version_info[0]<3 and (lambda x:x) or (lambda x:x.encode('latin1'))
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import symbol_database as _symbol_database
from google.protobuf import descriptor_pb2
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor.FileDescriptor(
  name='DQN_Agent.proto',
  package='',
  syntax='proto3',
  serialized_pb=_b('\n\x0f\x44QN_Agent.proto\"\xa5\x01\n\nAgent_Info\x12\x14\n\x0c\x41gent_Action\x18\x01 \x01(\x07\x12\x18\n\x10\x43urrent_Position\x18\x02 \x01(\x07\x12 \n\x18\x41gent_Trading_Instrument\x18\x03 \x01(\t\x12\x18\n\x10\x41\x63tion_Timestamp\x18\x04 \x01(\t\x12\x14\n\x0cLatestResult\x18\x05 \x01(\x08\x12\x15\n\rmsg_pipe_init\x18\x06 \x01(\x08\"\x88\x01\n\x07MD_Info\x12\x12\n\nlast_price\x18\x01 \x01(\x01\x12\x0e\n\x06volume\x18\x02 \x01(\x07\x12\x15\n\rmsg_pipe_init\x18\x03 \x01(\x08\x12\x14\n\x0cMD_Timestamp\x18\x04 \x01(\t\x12\x15\n\rMD_Instrument\x18\x05 \x01(\t\x12\x15\n\rMD_TradingDay\x18\x06 \x01(\tb\x06proto3')
)




_AGENT_INFO = _descriptor.Descriptor(
  name='Agent_Info',
  full_name='Agent_Info',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='Agent_Action', full_name='Agent_Info.Agent_Action', index=0,
      number=1, type=7, cpp_type=3, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='Current_Position', full_name='Agent_Info.Current_Position', index=1,
      number=2, type=7, cpp_type=3, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='Agent_Trading_Instrument', full_name='Agent_Info.Agent_Trading_Instrument', index=2,
      number=3, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='Action_Timestamp', full_name='Agent_Info.Action_Timestamp', index=3,
      number=4, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='LatestResult', full_name='Agent_Info.LatestResult', index=4,
      number=5, type=8, cpp_type=7, label=1,
      has_default_value=False, default_value=False,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='msg_pipe_init', full_name='Agent_Info.msg_pipe_init', index=5,
      number=6, type=8, cpp_type=7, label=1,
      has_default_value=False, default_value=False,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=20,
  serialized_end=185,
)


_MD_INFO = _descriptor.Descriptor(
  name='MD_Info',
  full_name='MD_Info',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='last_price', full_name='MD_Info.last_price', index=0,
      number=1, type=1, cpp_type=5, label=1,
      has_default_value=False, default_value=float(0),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='volume', full_name='MD_Info.volume', index=1,
      number=2, type=7, cpp_type=3, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='msg_pipe_init', full_name='MD_Info.msg_pipe_init', index=2,
      number=3, type=8, cpp_type=7, label=1,
      has_default_value=False, default_value=False,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='MD_Timestamp', full_name='MD_Info.MD_Timestamp', index=3,
      number=4, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='MD_Instrument', full_name='MD_Info.MD_Instrument', index=4,
      number=5, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='MD_TradingDay', full_name='MD_Info.MD_TradingDay', index=5,
      number=6, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None, file=DESCRIPTOR),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=188,
  serialized_end=324,
)

DESCRIPTOR.message_types_by_name['Agent_Info'] = _AGENT_INFO
DESCRIPTOR.message_types_by_name['MD_Info'] = _MD_INFO
_sym_db.RegisterFileDescriptor(DESCRIPTOR)

Agent_Info = _reflection.GeneratedProtocolMessageType('Agent_Info', (_message.Message,), dict(
  DESCRIPTOR = _AGENT_INFO,
  __module__ = 'DQN_Agent_pb2'
  # @@protoc_insertion_point(class_scope:Agent_Info)
  ))
_sym_db.RegisterMessage(Agent_Info)

MD_Info = _reflection.GeneratedProtocolMessageType('MD_Info', (_message.Message,), dict(
  DESCRIPTOR = _MD_INFO,
  __module__ = 'DQN_Agent_pb2'
  # @@protoc_insertion_point(class_scope:MD_Info)
  ))
_sym_db.RegisterMessage(MD_Info)


# @@protoc_insertion_point(module_scope)
