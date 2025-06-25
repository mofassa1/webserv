#include "HttpRequest.hpp"
#include "Multiplexer.hpp"

ResponseInfos Client::POST()
{
    ResponseInfos response;
    std::string full_path = LocationMatch.directory + LocationMatch.path;

    //std::cout << MAGENTA << "FF THIIIIIIIIIIIS" << std::endl;
    std::string file_extension = getFileExtension(full_path);
    file_extension += ':';
    //std::cout << "file_extension: " << file_extension << std::endl;
    if (isCGI(file_extension, LocationMatch.cgi))
    {
        //std::cout << "THIS IS CGI IN POST" << std::endl;
        std::string path_cgi = LocationMatch.cgi[file_extension];
        
        return executeCGI(path_cgi, full_path);
    }
    std::ostringstream ss;
    ss << "<html><body><h1>File uploaded successfully</h1>";
    ss << "<p>Saved to: " << LocationMatch.upload_path << "</p>";
    ss << "</body></html>";

    response.status = OK;
    response.body = ss.str();
    response.contentType = "text/html";
    response.headers["Content-Type"] = "text/html";
    response.headers["Content-Length"] = to_string(response.body.size());

    return response;
}

ResponseInfos Client::deleteDir(const std::string &path)
{
    ResponseInfos response;
    std::ostringstream ss;

    DIR *dir = opendir(path.c_str());
    if (!dir)
    {
        response.status = FORBIDDEN;
        response.body = "<html><body><h1>Forbidden</h1><p>Unable to access the directory: " + path + "</p></body></html>";
        response.contentType = "text/html";
        response.headers["Content-Type"] = "text/html";
        response.headers["Content-Length"] = std::to_string(response.body.size());
        return response;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            std::string fullPath = path + "/" + entry->d_name;
            struct stat statbuf;

            if (stat(fullPath.c_str(), &statbuf) == -1)
            {
                closedir(dir);
                throw FORBIDDEN;
            }
            if (S_ISDIR(statbuf.st_mode))
            {
                ResponseInfos resp = deleteDir(fullPath);
                if (resp.status != OK)
                {
                    closedir(dir);
                    return resp;
                }
            }
            else
            {
                if (remove(fullPath.c_str()) != 0)
                {
                    closedir(dir);
                    response.status = FORBIDDEN;
                    response.body = "<html><body><h1>Forbidden</h1><p>Unable to delete file: " + fullPath + "</p></body></html>";
                    response.contentType = "text/html";
                    response.headers["Content-Type"] = "text/html";
                    response.headers["Content-Length"] = std::to_string(response.body.size());
                    return response;
                }
            }
        }
    }
    closedir(dir);
    if (rmdir(path.c_str()) == 0)
    {
        ss << "<html><body><h1>Directory Deleted</h1><p>" << path << " was successfully deleted.</p></body></html>";
        response.status = OK;
        response.body = ss.str();
        response.contentType = "text/html";
        response.headers["Content-Type"] = "text/html";
        response.headers["Content-Length"] = std::to_string(response.body.size());
        return response;
    }
    response.status = FORBIDDEN;
    response.body = "<html><body><h1>Forbidden</h1><p>Unable to delete directory: " + path + "</p></body></html>";
    response.contentType = "text/html";
    response.headers["Content-Type"] = "text/html";
    response.headers["Content-Length"] = std::to_string(response.body.size());
    return response;
}

ResponseInfos Client::DELETE()
{
    ResponseInfos response;
    struct stat statbuf;
    std::string full_path = LocationMatch.directory + LocationMatch.path;

    if (stat(full_path.c_str(), &statbuf) != 0)
        throw NOT_FOUND;
    if (S_ISREG(statbuf.st_mode))
    {
        if (std::remove(full_path.c_str()) != 0)
            throw FORBIDDEN;

        std::ostringstream ss;
        ss << "<html><body><h1>File Deleted</h1><p>" << full_path << " was successfully deleted.</p></body></html>";
        response.status = OK;
        response.body = ss.str();
    }
    else if (S_ISDIR(statbuf.st_mode))
        return deleteDir(full_path);
    else
        throw FORBIDDEN;
    response.contentType = "text/html";
    response.headers["Content-Type"] = "text/html";
    response.headers["Content-Length"] = to_string(response.body.size());

    return response;
}

ResponseInfos Client::GET()
{
    std::string full_path = LocationMatch.directory + LocationMatch.path;

    //std::cout << RED << "QuEEEEEEEEEEEERY" << httpRequest.getDecodedPath() << COLOR_RESET << std::endl;

    struct stat file_info;
    if (stat(full_path.c_str(), &file_info) != 0)
        throw NOT_FOUND;

    if (S_ISDIR(file_info.st_mode))
    {
        if (access(full_path.c_str(), R_OK | X_OK) != 0)
            throw FORBIDDEN;
        if (!LocationMatch.index_file.empty())
        {
            std::string index_path = full_path + "/" + LocationMatch.index_file;

            struct stat index_info;
            if (stat(index_path.c_str(), &index_info) == 0 && S_ISREG(index_info.st_mode))
            {
                if (access(index_path.c_str(), R_OK) == 0)
                    return generateResponse(RESPONSE_FILE, index_path, OK, LocationMatch);
                else
                    throw FORBIDDEN;
            }
        }
        if (LocationMatch.autoindex)
            return generateResponse(RESPONSE_DIRECTORY_LISTING, full_path, OK, LocationMatch);
        else
            throw FORBIDDEN;
    }
    if (S_ISREG(file_info.st_mode))
    {
        if (access(full_path.c_str(), R_OK) != 0)
            throw FORBIDDEN; // Forbidden
        // IF CGI
        std::string file_extension = getFileExtension(full_path);
        file_extension += ':';
        //std::cout << "file_extension: " << file_extension << std::endl;
        if (isCGI(file_extension, LocationMatch.cgi))
        {
            std::string path_cgi = LocationMatch.cgi[file_extension];
            return executeCGI(path_cgi, full_path);
        }
        return generateResponse(RESPONSE_FILE, full_path, OK, LocationMatch);
    }
    throw NOT_FOUND;
}