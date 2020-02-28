=============
Low Level API
=============

.. default-domain:: cpp
.. cpp:namespace:: json

There is a straight-forward, low level API that you may find suitable for
working with JSON documents. It follows a typical lexer / parser pattern. You
can manage the lex and parse steps separately with :class:`json::Scanner` and
:class:`json::Parser`, or you can you use the thin combination
:class:`json::LexerParser`.

To work with the parse stream from a text document:

.. code-block:: c++

    int DoParseStream(const std::string& content, std::ostream* log) {
      json::LexerScanner stream{};
      json::Error{};

      int status = stream.Init(&error);
      if(status < 0){
        (*log) << json::Error::ToString(error.code) << ": " << error.msg;
        return status;
      }

      stream.Begin(content);

      json::Event event{}
      while(stream.GetNextEvent(&event, &error) == 0){
        switch(event.typeno){
          // Emitted on the start of a json object (i.e. the '{' token)
          case json::Event::OBJECT_BEGIN:

          // Emitted when a key is parsed. The event.token.spelling contains the
          // content of the key as a string literal (i.e. including the double
          // quotes)
          case json::Event::OBJECT_KEY:

          // Emitted on the end of a json object (i.e. the '}' token)
          case json::Event::OBJECT_END:

          // Emitted on the start of a json list (i.e. the '[' token)
          case json::Event::LIST_BEGIN:

          // Emitted on the end of a json list (i.e. the ']' token)
          case json::Event::LIST_END:

          // Emitted on any value literal including numeric, string, null, or
          // boolean.
          case json::Event::VALUE_LITERAL:
            break;
        }
      }

      if(error.code != json::Error::LEX_INPUT_FINISHED){
        (*log) << json::Error::ToString(error.code) << ": " << error.msg;
        return -1;
      } else {
        return 0;
      }
    }

To work with a token and event stream directly from a text document:

.. code-block:: c++

    int DoTokenStream(const std::string& content, std::ostream* log) {
      json::Scanner scanner{};
      json::Parser  parser{};
      json::Error{};

      int status = scanner.Init(&error);
      if(status < 0){
        (*log) << json::Error::ToString(error.code) << ": " << error.msg;
        return -1;
      }

      scanner.Begin(content);

      json::Token token{};
      json::Event event{};
      while(scanner.Pump(&token, &error) == 0){
        int status = parser.HandleToken(token, &event, &error);
        // error
        if(status < 0){
          (*log) << json::Error::ToString(error.code) << ": " << error.msg;
          return -1;
        } else if(status > 0){
          // An actionable event has occured, do something with the event
        } else {
          // No actionable event, but you can do something with the token
          // if you want. This means the token is either whitespace, colon,
          // or comma.
        }
      }

      if(error.code != json::Error::LEX_INPUT_FINISHED){
        return -1;
      } else {
        return 0;
      }
    }
