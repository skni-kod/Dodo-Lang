#include "LexicalAnalysis.hpp"

list_of_tokens* list_of_tokens::list_of_tokens_ = nullptr;

bool list_of_tokens::look_for_str_table(std::string str, std::string tab[], int R) {

    bool ret = false;

    for (int i = 0; i < R; i++) {
        if (str == tab[i]) {
            return true;
        }
    }

    return false;
}

list_of_tokens* list_of_tokens::get_instance() {
    if (list_of_tokens_ == nullptr) {
        list_of_tokens_ = new list_of_tokens();
    }

    return list_of_tokens_;
}

list_of_tokens::~list_of_tokens() {
    //if (list_of_tokens_ != nullptr)
    //delete list_of_tokens_;
}

std::vector<ProgramLine> list_of_tokens::get_list_of_tokens() {
    return token_list;
}

std::vector<ProgramLine> list_of_tokens::analize_file(std::fstream& file, std::string name) {
    std::string line;
    std::string word;
    std::string library;
    std::string operand;
    std::string help;

    int words_num;
    int counter = 0;
    char ch;


    bool probably_operand;
    bool word_under_construction;
    bool literal_under_construction;
    bool found_sth = false;
    bool big_comment = false;

    int size;
    int index, index2, index3;

    std::vector<ProgramLine> internal_token_list;

    while (std::getline(file, line)) {
        ProgramLine l;
        added_file_info f_name;
        size = line.length();
        word_under_construction = false;
        literal_under_construction = false;
        probably_operand = false;
        //this for analize one line of program and looks for tokens

        words_num = 0;
        for (int i = 0; i < size; i++) {
            ch = line[i];

            //means that this includes external files or sth
            if (ch == '#') {
                //checks if this line import files
                index3 = line.find("imp");
                if (index3 > -1) {
                    index3 = line.find("\"");
                    if (index3 < 0) {
                        break;
                    }

                    //cut line from " to end
                    library = line.substr(index3 + 1);

                    //looks for closing character
                    index3 = library.find("\"");

                    //case when there is another "
                    if (index3 >= 0) {
                        library = library.substr(0, index3);
                    }
                    else //cut to the end
                    {
                        library = library.substr(0, index3);
                    }
                    f_name.file_name = library;
                    f_name.master_file = name;
                    f_name.number_line = counter;
                    file_names.push_back(f_name);
                }


                //breaks for, always if # is inside
                break;
            }

            //it look for end of multiline comment
            //if not break for
            //if true sets index after end of multiline comment
            if (big_comment) {
                index = line.find("*/");
                if (index != -1) {
                    i = index + 2;
                    big_comment = false;
                }
                else {
                    break;
                }
            }

            //checking if comment
            //if true break for
            if ((ch == '/' && i < size - 1) && (line[i + 1] == '/' || line[i + 1] == '*')) {
                help = ch;
                help += line[i + 1];

                if (help == "/*") {
                    big_comment = true;
                }

                break;
            }


            //constructing word - until find blank space or operand
            //or constructing numeric literalType - it's starts with number
            if ((isalpha(ch) || ch == '_') || word_under_construction || literal_under_construction || isdigit(ch)) {
                //sets what kind of build it is
                if (isalpha(ch) && !literal_under_construction) {
                    word_under_construction = true;
                }
                else if (isdigit(ch) && !word_under_construction) {
                    literal_under_construction = true;
                }

                //check if it is operand
                for (int j = 0; j < number_of_operands; j++) {
                    help = ch;
                    if (help == list_of_operands[j]) {
                        probably_operand = true;

                        if (help == "." && literal_under_construction) {
                            probably_operand = false;
                        }

                        break;
                    }

                }
                //if it is not operand and it is not white space then add char to creating word
                if (!probably_operand && (ch != ' ' && ch != '\t')) {
                    word += ch;
                }
                else {

                    //this works when lexer found word that is not operand or comment

                    //checking if it is a numeric literalType
                    if (literal_under_construction) {
                        l.add_token(7, word, true);
                        word = "";
                        words_num++;
                    }

                    //checking if it is keyword
                    for (int k = 0; k < numer_of_keywords; k++) {
                        if (word == list_of_keywords[k]) {
                            l.add_token(keyword, word);
                            words_num++;
                            word = "";
                            break;
                        }
                    }
                    //if it is not keyword and it is not literalType then it is identifier
                    if (word != "") {
                        l.add_token(identifier, word);
                        words_num++;
                        word = "";
                    }

                    word_under_construction = false;
                    literal_under_construction = false;

                    if (ch != ' ' && ch != '\t') {
                        operand = ch;
                    }
                }
            }


            //checking if the ch is operand
            //and dealing with operand at the end of the keyword or identifier
            if (!word_under_construction || !isalpha(ch)) {
                help = ch;

                //checs if it's structural operand
                switch (ch) {

                    case '\'':
                        index2 = 0;
                        index2 = line.find("\'", i + 1);
                        if (index2 != -1) {
                            help = line.substr(i, index2 - i + 1);
                            //adding character literalType
                            l.add_token(7, help, character);
                            i = index2 + 1;
                        }
                        else {
                            help = line.substr(i, size - i + 1);
                            //probably missing second '
                            l.add_token(8, help);
                            help = "";
                        }
                        words_num++;
                        //help = "";
                        break;

                    case '\"':
                        index2 = line.find("\"", i + 1);
                        if (index2 != -1) {
                            help = line.substr(i, index2 - i + 1);
                            //adding string literalType
                            l.add_token(7, help, string_type);
                            i = index2 + 1;
                        }
                        else {
                            help = line.substr(i, size - i + 1);
                            //probably missing second "
                            l.add_token(8, help);
                            help = "";
                        }

                        words_num++;
                        //help = "";
                        break;
                }

                if (help == "") {
                    break;
                }

                operand = ch;
                probably_operand = false;

                //check if its in list of operands
                probably_operand = look_for_str_table(operand, list_of_operands, number_of_operands);

                //if it is the last character in line add it
                if (probably_operand && i == size - 1) {
                    if (operand == ",") {
                        l.add_token(3, ",");
                    }
                    else {
                        l.add_token(1, operand);
                    }
                    words_num++;
                }

                //if it is not the last character in line check if it is two characters operand
                if (probably_operand && i < size - 1) {
                    help = operand;
                    help += line[i + 1];

                    probably_operand = look_for_str_table(help, list_of_operands, number_of_operands);

                    //add operand - if it has two charactes avoid checking the next operand
                    if (probably_operand) {
                        l.add_token(1, help);
                        words_num++;
                        i++;
                    }
                    else {
                        l.add_token(1, operand);
                        words_num++;
                    }
                }
            }
        }

        if (word_under_construction) {
            if (look_for_str_table(word, list_of_keywords, numer_of_keywords)) {
                l.add_token(keyword, word);
            }
            else {
                l.add_token(identifier, word);
            }
        }

        if (literal_under_construction) {
            l.add_token(7, word, true);
        }

        word = "";

        l.line_number = counter;
        //if line is important add it to list of lines
        if (words_num != 0) {
            l.l_size = l.line.size();
            token_list.push_back(l);
            internal_token_list.push_back(l);
        }


        //line number couter
        counter++;

    }
    return internal_token_list;
}

void list_of_tokens::lexical_analize(const std::string start_file_name) {
    std::string helper;
    std::fstream file;
    ProgramPage p;
    added_file_info add;

    //clearing everything
    file_names.clear();
    f_token_list.clear();
    token_list.clear();
    f_size = 0;
    was_errors = false;

    add.file_name = start_file_name;
    add.master_file = "none";
    add.number_line = 0;

    this->file_names.push_back(add);

    while (file_names.size() > 0) {
        helper = file_names[0].file_name;
        file.open(helper.c_str(), std::fstream::in);

        if (file) {
            //file exists - analize it
            ProgramLine l;
            l.add_token(file_begin, helper);
            l.line_number = 0;
            token_list.push_back(l);
            p.page = analize_file(file, helper);
            p.file_name = helper;
            p.p_size = p.page.size();
            f_token_list.push_back(p);
            f_size++;
            file.close();
        }
        else {
            //file don't exists - display error
            was_errors = true;
            std::cout << "found error ";
            std::cout << "inside: " << file_names[0].master_file;
            std::cout << " line: " << file_names[0].number_line << std::endl;
            std::cout << "error info: ";
            std::cout << "WHERE THE **** IS THAT PIECE OF **** " << helper << "!???" << std::endl;
        }
        file_names.erase(file_names.begin());
    }


    size = token_list.size();
}

void list_of_tokens::list_of_tokens_print() {
    int size = token_list.size();
    for (int i = 0; i < size; i++) {
        std::cout << token_list[i].line_number << ". " << token_list[i] << std::endl;
    }
}

