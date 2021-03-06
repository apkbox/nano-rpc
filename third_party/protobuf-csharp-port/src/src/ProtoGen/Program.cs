#region Copyright notice and license
// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// http://github.com/jskeet/dotnet-protobufs/
// Original C++/Java/Python code:
// http://code.google.com/p/protobuf/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#endregion

using System;
using System.Collections.Generic;
using Google.ProtocolBuffers.DescriptorProtos;

namespace Google.ProtocolBuffers.ProtoGen {
  /// <summary>
  /// Entry point for the Protocol Buffers generator.
  /// </summary>
  class Program {
    static int Main(string[] args) {
      try {
        // Hack to make sure everything's initialized
        DescriptorProtoFile.Descriptor.ToString();
        GeneratorOptions options = ParseCommandLineArguments(args);

        IList<string> validationFailures;
        if (!options.TryValidate(out validationFailures)) {
          // We've already got the message-building logic in the exception...
          InvalidOptionsException exception = new InvalidOptionsException(validationFailures);
          Console.WriteLine(exception.Message);
          return 1;
        }

        Generator generator = Generator.CreateGenerator(options);
        generator.Generate();
        return 0;
      } catch (Exception e) {
        Console.Error.WriteLine("Error: {0}", e.Message);
        Console.Error.WriteLine();
        Console.Error.WriteLine("Detailed exception information: {0}", e);
        return 1;
      }
    }

    private static GeneratorOptions ParseCommandLineArguments(string[] args) {
      GeneratorOptions options = new GeneratorOptions();
      //string baseDir = "c:\\Users\\Jon\\Documents\\Visual Studio 2008\\Projects\\ProtocolBuffers";
      //options.OutputDirectory = baseDir + "\\tmp";
      //options.InputFiles = new[] { baseDir + "\\protos\\nwind-solo.protobin" };

      options.OutputFileSuffix = string.Empty;
      options.OutputDirectory = string.Empty;

      List<string> arguments = new List<string>();
      foreach (var s in args)
      {
          if (s.StartsWith("--output-extension=")) {
              options.OutputFileSuffix = s.Split(new[] { '=' }, 2)[1].Trim();
          } else if (s.StartsWith("--output-directory=")) {
              options.OutputDirectory = s.Split(new[] { '=' }, 2)[1].Trim();
          } else {
              arguments.Add(s);
          }
      }

      if (options.OutputFileSuffix.Length == 0)
          options.OutputFileSuffix = ".cs";

      if (options.OutputDirectory.Length == 0)
          options.OutputDirectory = ".";

      options.InputFiles = arguments;
      return options;
    }
  }
}