#include "Multiplexer.hpp"

ResponseInfos Client::POST()
{
    ResponseInfos response;

    std::ostringstream ss;
    ss << "<html><body><h1>File uploaded successfully</h1>";
    ss << "<p>Saved to: " << LocationMatch.upload_path << "</p>";
    ss << "</body></html>";

    response.status = 200;
    response.body = ss.str();
    response.contentType = "text/html";
    response.headers["Content-Type"] = "text/html";
    response.headers["Content-Length"] = to_string(response.body.size());

    return response;
}


ResponseInfos Client::deleteDir(const std::string path)
{
    ResponseInfos response;
    std::ostringstream ss;

    ss << "<html><body><h1>File Deleted</h1><p>" << path << " was successfully deleted.</p></body></html>";
    
    response.status = 200;
    response.body = ss.str();
    response.contentType = "text/html";
    response.headers["Content-Type"] = "text/html";
    response.headers["Content-Length"] = to_string(response.body.size());

    return response;

    // to be handled
   
    //     DIR *dir = opendir(path.c_str());
//     if (!dir)
//         throw FORBIDDEN;

//     struct dirent *entry;
//     while ((entry = readdir(dir)) != NULL)
//     {
//         if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
//         {
//             std::string fullPath = path + "/" + entry->d_name;
//             struct stat statbuf;
//             if (stat(fullPath.c_str(), &statbuf) == -1)
//             {
//                 closedir(dir);
//                 throw FORBIDDEN;
//             }
//             if (S_ISDIR(statbuf.st_mode))
//             {
//                 ResponseInfos resp = deleteDir(fullPath);
//                 if (resp.status != NO_CONTENT)
//                 {
//                     closedir(dir);
//                     return resp; 
//                 }
//             }
//             else
//             {
//                 if (remove(fullPath.c_str()) != 0)
//                 {
//                     closedir(dir);
//                     throw FORBIDDEN; 
//                 }
//             }
//         }
//     }
//     closedir(dir);
//     if (rmdir(path.c_str()) == 0)
    return generateResponse(RESPONSE_DELETE, path, 200, LocationMatch);

//     throw FORBIDDEN;
}


ResponseInfos  Client::DELETE()
{
    ResponseInfos response;
    struct stat statbuf;
    std::string full_path = LocationMatch.directory + LocationMatch.path; // Build the full file path

    if (stat(full_path.c_str(), &statbuf) != 0)
        throw NOT_FOUND;
    if (S_ISREG(statbuf.st_mode))
    {
        if (std::remove(full_path.c_str()) != 0)
            throw FORBIDDEN;

        std::ostringstream ss;
        ss << "<html><body><h1>File Deleted</h1><p>" << full_path << " was successfully deleted.</p></body></html>";
        response.status = 200;
        response.body = ss.str();
    }
    else if (S_ISDIR(statbuf.st_mode))
    {
        // You may want to restrict directory deletion
        return deleteDir(full_path);
        throw FORBIDDEN;
    }
    else
    {
        throw FORBIDDEN;
    }
    response.contentType = "text/html";
    response.headers["Content-Type"] = "text/html";
    response.headers["Content-Length"] = to_string(response.body.size());

    return response;
}


ResponseInfos Client::GET()
{
    std::string full_path = LocationMatch.directory + LocationMatch.path; // Build the full file path
    std::cout<< GREEN << full_path << COLOR_RESET << std::endl;

    struct stat file_info;
    if (stat(full_path.c_str(), &file_info) != 0)
        throw NOT_FOUND;

    std::cerr << RED << "HERE HERE" << COLOR_RESET << std::endl;
    if (S_ISDIR(file_info.st_mode))
    {
        std::cerr << RED << "HERE 1" << COLOR_RESET << std::endl;
        if (!LocationMatch.index_file.empty())
        {
            std::cerr << RED << "HERE" << COLOR_RESET << std::endl;
            std::string index_path = full_path + "/" + LocationMatch.index_file;

            struct stat index_info;
            if (stat(index_path.c_str(), &index_info) == 0 && S_ISREG(index_info.st_mode))
                return generateResponse(RESPONSE_FILE, index_path, 200, LocationMatch);
        }
        if (LocationMatch.autoindex)
            return generateResponse(RESPONSE_DIRECTORY_LISTING, full_path, 200, LocationMatch);
        else
            throw FORBIDDEN; // forbidden
    }
    if (S_ISREG(file_info.st_mode))
    {
        // IF CGI
        std::string file_extension = getFileExtension(full_path);
        file_extension += ':';
        if (isCGI(file_extension, LocationMatch.cgi)){
        std::string path_cgi = LocationMatch.cgi[file_extension];
            return executeCGI(path_cgi, full_path);
        }
        ////////////
        
        if (access(full_path.c_str(), R_OK) != 0)
            throw FORBIDDEN; // Forbidden
        return generateResponse(RESPONSE_FILE, full_path, 200, LocationMatch);
    }
    throw NOT_FOUND;
}