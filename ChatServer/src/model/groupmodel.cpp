#include <muduo/base/Logging.h>

#include "groupmodel.hpp"
#include "connectionpool.hpp"

const int MAX_SQL_SIZE = 1024;

bool GroupModel::createGroup(Group &newGroup){

    char sql[MAX_SQL_SIZE] = {'\0'};
    sprintf(sql, "insert into allgroup(groupname, groupdesc) values('%s', '%s')",
            newGroup.getGroupName().c_str(), newGroup.getGroupDesc().c_str());

    std::shared_ptr<MySQL> curConn = ConnectionPool::instance()->getConn();
    if(curConn != nullptr){
        if (curConn->update(sql))
        {
            newGroup.setId(mysql_insert_id(curConn->getConnection()));
            return true;
        }
    }
    return false;
}

bool GroupModel::addToGroup(int userId, int groupId, std::string groupRole){

    char sql[MAX_SQL_SIZE] = {'\0'};
    sprintf(sql, "insert into groupuser values('%d','%d','%s')",
            groupId, userId, groupRole.c_str());

            
    std::shared_ptr<MySQL> curConn = ConnectionPool::instance()->getConn();
    if(curConn != nullptr){
        if (curConn->update(sql))
        {
            return true;
        }
    }
    return false;
}

std::vector<Group> GroupModel::queryMyGroups(int userId){

    std::vector<Group> ret;
    Group tempGroup;
    GroupUser tempGroupUser;

    char sql[MAX_SQL_SIZE] = {'\0'};
    sprintf(sql, "select a.id,a.groupname,a.groupdesc from allgroup a inner join groupuser b on b.groupid = a.id where b.userid = '%d'",
            userId);

    std::shared_ptr<MySQL> curConn = ConnectionPool::instance()->getConn();
    if(curConn != nullptr){
        MYSQL_RES *res = curConn->query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW resRow;
            while ((resRow = mysql_fetch_row(res)) != nullptr)
            {
                tempGroup.setId(atoi(resRow[0]));
                tempGroup.setGroupName(resRow[1]);
                tempGroup.setGroupDesc(resRow[2]);

                ret.push_back(tempGroup);
            }
            mysql_free_result(res);
        }

        for (auto &group : ret)
        {
            sprintf(sql, "select a.id,a.name,a.state,b.grouprole from user a inner join groupuser b on b.userid = a.id where b.groupid = '%d'",
                    group.getId());

            MYSQL_RES *res = curConn->query(sql);
            if (res != nullptr)
            {
                MYSQL_ROW resRow;
                while ((resRow = mysql_fetch_row(res)) != nullptr)
                {
                    tempGroupUser.setId(atoi(resRow[0]));
                    tempGroupUser.setName(resRow[1]);
                    tempGroupUser.setState(resRow[2]);
                    tempGroupUser.setRole(resRow[3]);

                    group.getGroupUsers().push_back(tempGroupUser);
                }
                mysql_free_result(res);
            }
        }
    }
    return ret;
}

std::vector<int> GroupModel::queryThisGroup(int userId, int groupId){

    std::vector<int> ret;

    char sql[MAX_SQL_SIZE] = {'\0'};
    sprintf(sql, "select userid from groupuser where groupid = '%d' and userid != '%d'",
            groupId, userId);

    
    std::shared_ptr<MySQL> curConn = ConnectionPool::instance()->getConn();
    if(curConn != nullptr){
        MYSQL_RES *res = curConn->query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW resRow;
            while ((resRow = mysql_fetch_row(res)) != nullptr)
            {
                ret.push_back(atoi(resRow[0]));
            }
            mysql_free_result(res);
        }
    }
    return ret;
}