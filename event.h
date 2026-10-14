#include <vector>
enum parameters {none, create, append, extract, list};

class event{
private:
    int argc;
    std::vector<std::string> arguments;
    std::fstream file;
    parameters options;
    bool verbose;
    bool addEmptyBlocks;
protected:
    void COption()
    {
        options = parameters::create;
        addEmptyBlocks=true;
        if (arguments[1].find('v')!=std::string::npos) verbose=true;
        file.open(arguments[2],std::ios::binary | std::ios::out);
        if(!file.is_open())
        {
            std::cerr<<"reading error1\n"<<arguments[2];
            return;
        }
    }

    void AOption()
    {
        options = parameters::append;
        if (arguments[1].find('v')!=std::string::npos) verbose=true;
        file.open(arguments[2],std::ios::binary | std::ios::in | std::ios::out);
        if(!file.is_open())
        {
            std::cerr<<"reading error1\n"<<arguments[2];
            return;
        }
    }

    void EOption()
    {
        options = parameters::extract;
        if (arguments[1].find('v')!=std::string::npos) verbose=true;
        file.open(arguments[2], std::ios::binary | std::ios::in);
        if(!file.is_open())
        {
            std::cerr<<"reading error1\n"<<arguments[2];
            return;
        }
    }
    void LOption()
    {
        options = parameters::list;
        if (arguments[1].find('v')!=std::string::npos) verbose=true;
        file.open(arguments[2], std::ios::binary | std::ios::in);
        if(!file.is_open())
        {
            std::cerr<<"reading error1\n"<<arguments[2];
            return;
        }
    }
public:
    event(int p_argc, char** p_argv): argc(p_argc), options(parameters::none), verbose(false), addEmptyBlocks(false){
    for (int i = 0; p_argv[i] != nullptr; ++i) {
        arguments.emplace_back(p_argv[i]);
    }}

    

    void Parser()
    {
        if((argc<3))
        {
            std::cerr<<"wrong command! "<< argc;
            return;
        }
        if (arguments[1].find('c')!=std::string::npos)
        {
            if(arguments[1].find('r')!=std::string::npos && arguments[1].find('x')!=std::string::npos)
                {std::cerr<<"incorrect command!\n"<< arguments[1];
                    return;
                }
            COption();
        }
        if (arguments[1].find('r')!=std::string::npos)
        {
            if(arguments[1].find('c')!=std::string::npos && arguments[1].find('x')!=std::string::npos)
                {std::cerr<<"incorrect command!\n"<< arguments[1];
                    return;
                }
            AOption();
        }
        if (arguments[1].find('x')!=std::string::npos)
        {
            if(arguments[1].find('r')!=std::string::npos && arguments[1].find('c')!=std::string::npos)
                {std::cerr<<"incorrect command!\n"<< arguments[1];
                    return;
                }
            EOption();
        }
        if(arguments[1].find('t')!=std::string::npos)
        {
            LOption();
        }
        if(arguments[1].find('f')!=std::string::npos)
            std::cout << arguments[2] << std::endl;
    }

    parameters getOption(){
        return options;
    }
    std::fstream& getStream()
    {
        return file;
    }
    bool checkVerbose()
    {
        return verbose;
    }
    bool checkEmptuBlocks()
    {
        return addEmptyBlocks;
    }
    int getArgc()
    {
        return argc;
    }
    std::string getArgument(int n)
    {
        return arguments[n];
    }
    bool check3Argc()
    {
        return argc==3;
    }
    
    ~event()=default;
};