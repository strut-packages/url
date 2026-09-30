error UrlError {
    string message;
    int code;
}

struct Url {
    string? scheme;
    string? authority;
    string? userinfo;
    string? hostname;
    string? port;
    bool host_is_ip_literal;
    string path;
    string? query;
    string? fragment;
}

struct UrlQueryParam {
    string name;
    string? value;
}

function url_internal_alpha(uint_8 value) -> bool {
    return (value >= 65 && value <= 90) || (value >= 97 && value <= 122);
}

function url_internal_digit(uint_8 value) -> bool {
    return value >= 48 && value <= 57;
}

function url_internal_hex(uint_8 value) -> bool {
    return url_internal_digit(value)
    || (value >= 65 && value <= 70)
    || (value >= 97 && value <= 102);
}

function url_internal_unreserved(uint_8 value) -> bool {
    return url_internal_alpha(value)
    || url_internal_digit(value)
    || value == 45
    || value == 46
    || value == 95
    || value == 126;
}

function url_internal_sub_delim(uint_8 value) -> bool {
    return value == 33
    || value == 36
    || value == 38
    || value == 39
    || value == 40
    || value == 41
    || value == 42
    || value == 43
    || value == 44
    || value == 59
    || value == 61;
}

function url_internal_scheme_byte(uint_8 value) -> bool {
    return url_internal_alpha(value)
    || url_internal_digit(value)
    || value == 43
    || value == 45
    || value == 46;
}

function url_internal_slice(bytes input, int_64 begin, int_64 end) -> string {
    return input.slice(begin, end).to_string();
}

function url_internal_optional(string? value) -> string {
    return value ?? "";
}

function url_internal_validate_component(bytes input, int component) -> void : UrlError {
    int_64 length := input.length();
    int_64 index := 0;
    while (index < length) {
        uint_8 value := input[index];
        if (value == 37) {
            if (index + 2 >= length || !url_internal_hex(input[index + 1]) || !url_internal_hex(input[index + 2])) {
                throw UrlError { message: "malformed percent escape", code: 3 };
            }
            index = index + 3;
            continue;
        }
        bool allowed := url_internal_unreserved(value) || url_internal_sub_delim(value);
        if (component == 1) {
            allowed = allowed || value == 58 || value == 64 || value == 91 || value == 93;
        } else if (component == 2 || component == 3) {
            allowed = allowed || value == 58 || value == 64 || value == 47;
            if (component == 3) {
                allowed = allowed || value == 63;
            }
        } else if (component == 4 || component == 6) {
            allowed = allowed || value == 58;
        }
        if (!allowed || value > 127) {
            throw UrlError { message: "character is not allowed in RFC 3986 URI component", code: 2 };
        }
        index++;
    }
    return;
}

function url_internal_validate_port(bytes input) -> void : UrlError {
    for (int_64 index := 0; index < input.length(); index++) {
        if (!url_internal_digit(input[index])) {
            throw UrlError { message: "port must contain only digits", code: 4 };
        }
    }
    return;
}

function url_parse(string text) -> Url : UrlError {
    bytes input := bytes.from_string(text);
    int_64 length := input.length();
    int_64 main_end := length;
    string? fragment := null;
    for (int_64 index := 0; index < length; index++) {
        if (input[index] == 35) {
            main_end = index;
            fragment = url_internal_slice(input, index + 1, length);
            break;
        }
    }

    int_64 hierarchy_end := main_end;
    string? query := null;
    for (int_64 index := 0; index < main_end; index++) {
        if (input[index] == 63) {
            hierarchy_end = index;
            query = url_internal_slice(input, index + 1, main_end);
            break;
        }
    }

    string? scheme := null;
    int_64 cursor := 0;
    int_64 colon := -1;
    for (int_64 index := 0; index < hierarchy_end; index++) {
        if (input[index] == 58) {
            colon = index;
            break;
        }
        if (input[index] == 47) {
            break;
        }
    }
    if (colon >= 0) {
        if (colon == 0 || !url_internal_alpha(input[0])) {
            throw UrlError { message: "invalid URI scheme", code: 1 };
        }
        for (int_64 index := 1; index < colon; index++) {
            if (!url_internal_scheme_byte(input[index])) {
                throw UrlError { message: "invalid URI scheme", code: 1 };
            }
        }
        scheme = url_internal_slice(input, 0, colon);
        cursor = colon + 1;
    }

    string? authority := null;
    string? userinfo := null;
    string? hostname := null;
    string? port := null;
    bool host_is_ip_literal := false;
    if (cursor + 1 < hierarchy_end && input[cursor] == 47 && input[cursor + 1] == 47) {
        int_64 authority_start := cursor + 2;
        int_64 authority_end := authority_start;
        while (authority_end < hierarchy_end && input[authority_end] != 47) {
            authority_end++;
        }
        authority = url_internal_slice(input, authority_start, authority_end);
        url_internal_validate_component(input.slice(authority_start, authority_end), 1);

        int_64 host_start := authority_start;
        int_64 at := -1;
        int at_count := 0;
        for (int_64 index := authority_start; index < authority_end; index++) {
            if (input[index] == 64) {
                at = index;
                at_count++;
            }
        }
        if (at_count > 1) {
            throw UrlError { message: "authority contains multiple userinfo delimiters", code: 4 };
        }
        if (at >= 0) {
            userinfo = url_internal_slice(input, authority_start, at);
            url_internal_validate_component(input.slice(authority_start, at), 4);
            host_start = at + 1;
        }

        if (host_start < authority_end && input[host_start] == 91) {
            int_64 close := -1;
            for (int_64 index := host_start + 1; index < authority_end; index++) {
                if (input[index] == 93) {
                    close = index;
                    break;
                }
            }
            if (close < 0) {
                throw UrlError { message: "unterminated bracketed IP literal", code: 4 };
            }
            hostname = url_internal_slice(input, host_start + 1, close);
            url_internal_validate_component(input.slice(host_start + 1, close), 6);
            host_is_ip_literal = true;
            if (close + 1 < authority_end) {
                if (input[close + 1] != 58) {
                    throw UrlError { message: "unexpected content after bracketed IP literal", code: 4 };
                }
                port = url_internal_slice(input, close + 2, authority_end);
                url_internal_validate_port(input.slice(close + 2, authority_end));
            }
        } else {
            int_64 separator := -1;
            int colon_count := 0;
            for (int_64 index := host_start; index < authority_end; index++) {
                if (input[index] == 58) {
                    separator = index;
                    colon_count++;
                }
            }
            if (colon_count > 1) {
                throw UrlError { message: "IP literals must be enclosed in brackets", code: 4 };
            }
            for (int_64 index := host_start; index < authority_end; index++) {
                if (input[index] == 91 || input[index] == 93) {
                    throw UrlError { message: "brackets are only valid around an IP literal", code: 4 };
                }
            }
            if (separator >= 0) {
                hostname = url_internal_slice(input, host_start, separator);
                url_internal_validate_component(input.slice(host_start, separator), 5);
                port = url_internal_slice(input, separator + 1, authority_end);
                url_internal_validate_port(input.slice(separator + 1, authority_end));
            } else {
                hostname = url_internal_slice(input, host_start, authority_end);
                url_internal_validate_component(input.slice(host_start, authority_end), 5);
            }
        }
        cursor = authority_end;
    }

    string path := url_internal_slice(input, cursor, hierarchy_end);
    url_internal_validate_component(bytes.from_string(path), 2);
    if (query != null) {
        url_internal_validate_component(bytes.from_string(url_internal_optional(query)), 3);
    }
    if (fragment != null) {
        url_internal_validate_component(bytes.from_string(url_internal_optional(fragment)), 3);
    }

    return Url {
        scheme: scheme,
        authority: authority,
        userinfo: userinfo,
        hostname: hostname,
        port: port,
        host_is_ip_literal: host_is_ip_literal,
        path: path,
        query: query,
        fragment: fragment
    };
}

function url_stringify(Url value) -> string {
    result := "";
    if (value.scheme != null) {
        result = result + (value.scheme ?? "") + ":";
    }
    if (value.authority != null) {
        result = result + "//" + (value.authority ?? "");
    }
    result = result + value.path;
    if (value.query != null) {
        result = result + "?" + (value.query ?? "");
    }
    if (value.fragment != null) {
        result = result + "#" + (value.fragment ?? "");
    }
    return result;
}

function url_internal_remove_dot_segments(string path) -> string {
    bytes input := bytes.from_string(path);
    int_64 length := input.length();
    bool absolute := length > 0 && input[0] == 47;
    bool trailing_slash := length > 0 && input[length - 1] == 47;
    string[] segments := [];
    int_64 begin := 0;
    while (begin <= length) {
        int_64 end := begin;
        while (end < length && input[end] != 47) {
            end++;
        }
        string segment := url_internal_slice(input, begin, end);
        if (segment == "..") {
            if (segments.length() > 0 && !(absolute && segments.length() == 1)) {
                segments.pop();
            }
            if (end == length) {
                trailing_slash = true;
            }
        } else if (segment == ".") {
            if (end == length) {
                trailing_slash = true;
            }
        } else {
            segments.push(segment);
        }
        if (end == length) {
            break;
        }
        begin = end + 1;
    }

    result := "";
    bool first := true;
    for (segment : segments) {
        if (!first) {
            result = result + "/";
        }
        result = result + segment;
        first = false;
    }
    if (trailing_slash && (result == "" || bytes.from_string(result)[bytes.from_string(result).length() - 1] != 47)) {
        result = result + "/";
    }
    return result;
}

function url_internal_merge_path(Url base, string reference_path) -> string {
    if (base.authority != null && base.path == "") {
        return "/" + reference_path;
    }
    bytes path := bytes.from_string(base.path);
    int_64 slash := -1;
    for (int_64 index := 0; index < path.length(); index++) {
        if (path[index] == 47) {
            slash = index;
        }
    }
    if (slash < 0) {
        return reference_path;
    }
    return path.slice(0, slash + 1).to_string() + reference_path;
}

function url_internal_compose(
string? scheme,
string? authority,
string path,
string? query,
string? fragment
) -> string {
    result := "";
    if (scheme != null) {
        result = result + url_internal_optional(scheme) + ":";
    }
    if (authority != null) {
        result = result + "//" + url_internal_optional(authority);
    }
    result = result + path;
    if (query != null) {
        result = result + "?" + url_internal_optional(query);
    }
    if (fragment != null) {
        result = result + "#" + url_internal_optional(fragment);
    }
    return result;
}

function url_resolve(Url base, string reference) -> Url : UrlError {
    if (base.scheme == null) {
        throw UrlError { message: "base URI must have a scheme", code: 6 };
    }
    Url relative := url_parse(reference);
    string? target_scheme := base.scheme;
    string? target_authority := null;
    string target_path := "";
    string? target_query := null;

    if (relative.scheme != null) {
        target_scheme = relative.scheme;
        target_authority = relative.authority;
        target_path = url_internal_remove_dot_segments(relative.path);
        target_query = relative.query;
    } else if (relative.authority != null) {
        target_authority = relative.authority;
        target_path = url_internal_remove_dot_segments(relative.path);
        target_query = relative.query;
    } else {
        target_authority = base.authority;
        if (relative.path == "") {
            target_path = base.path;
            if (relative.query != null) {
                target_query = relative.query;
            } else {
                target_query = base.query;
            }
        } else {
            bytes relative_path := bytes.from_string(relative.path);
            if (relative_path.length() > 0 && relative_path[0] == 47) {
                target_path = url_internal_remove_dot_segments(relative.path);
            } else {
                target_path = url_internal_remove_dot_segments(url_internal_merge_path(base, relative.path));
            }
            target_query = relative.query;
        }
    }

    return url_parse(url_internal_compose(
    target_scheme,
    target_authority,
    target_path,
    target_query,
    relative.fragment
    ));
}

function url_resolve_string(string base, string reference) -> string : UrlError {
    return url_stringify(url_resolve(url_parse(base), reference));
}

function url_percent_encode(bytes input) -> string {
    bytes hex := bytes.from_string("0123456789ABCDEF");
    bytes output := bytes(input.length() * 3);
    int_64 used := 0;
    for (int_64 index := 0; index < input.length(); index++) {
        uint_8 value := input[index];
        if (url_internal_unreserved(value)) {
            output[used] = value;
            used++;
        } else {
            output[used] = 37;
            output[used + 1] = hex[value / 16];
            output[used + 2] = hex[value % 16];
            used = used + 3;
        }
    }
    return output.slice(0, used).to_string();
}

function url_internal_hex_value(uint_8 value) -> uint_8 {
    if (value >= 48 && value <= 57) {
        value -= 48;
    } else if (value >= 65 && value <= 70) {
        value -= 55;
    } else {
        value -= 87;
    }
    return value;
}

function url_percent_decode(string text) -> bytes : UrlError {
    bytes input := bytes.from_string(text);
    bytes output := bytes(input.length());
    int_64 used := 0;
    int_64 index := 0;
    while (index < input.length()) {
        if (input[index] != 37) {
            output[used] = input[index];
            used++;
            index++;
            continue;
        }
        if (index + 2 >= input.length() || !url_internal_hex(input[index + 1]) || !url_internal_hex(input[index + 2])) {
            throw UrlError { message: "malformed percent escape", code: 3 };
        }
        uint_8 value := url_internal_hex_value(input[index + 1]);
        value *= 16;
        value += url_internal_hex_value(input[index + 2]);
        output[used] = value;
        used++;
        index = index + 3;
    }
    return output.slice(0, used);
}

function url_encode(string text) -> string {
    return url_percent_encode(bytes.from_string(text));
}

function url_decode(string text) -> string : UrlError {
    return url_percent_decode(text).to_string();
}

function url_parse_query(string query) -> UrlQueryParam[] : UrlError {
    bytes input := bytes.from_string(query);
    UrlQueryParam[] parameters := [];
    if (input.empty()) {
        return parameters;
    }
    int_64 begin := 0;
    while (begin <= input.length()) {
        int_64 end := begin;
        while (end < input.length() && input[end] != 38) {
            end++;
        }
        int_64 equals := -1;
        for (int_64 index := begin; index < end; index++) {
            if (input[index] == 61) {
                equals = index;
                break;
            }
        }
        string name := "";
        string? value := null;
        if (equals >= 0) {
            name = url_decode(url_internal_slice(input, begin, equals));
            value = url_decode(url_internal_slice(input, equals + 1, end));
        } else {
            name = url_decode(url_internal_slice(input, begin, end));
        }
        parameters.push(UrlQueryParam { name: name, value: value });
        if (end == input.length()) {
            break;
        }
        begin = end + 1;
    }
    return parameters;
}

function url_build_query(UrlQueryParam[] parameters) -> string {
    result := "";
    bool first := true;
    for (parameter : parameters) {
        if (!first) {
            result = result + "&";
        }
        result = result + url_encode(parameter.name);
        if (parameter.value != null) {
            result = result + "=" + url_encode(parameter.value ?? "");
        }
        first = false;
    }
    return result;
}
