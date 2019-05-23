/*
 *  Copyright (C) 2010 Felix Geyer <debfx@fobos.de>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 or (at your option)
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "DatabaseIcons.h"

#include "core/FilePath.h"

DatabaseIcons* DatabaseIcons::m_instance(nullptr);
const QUuid DatabaseIcons::CommonIconsUuid("959da7c1-a21c-40aa-a324-ce318c5a87c9");

const int DatabaseIcons::ExpiredIconIndex(45);
const int DatabaseIcons::SharedIconIndex(1);
const int DatabaseIcons::UnsharedIconIndex(45);

// clang-format off
const QList<QPair<QUuid, QString>> DatabaseIcons::m_commonIcons = {
    {"a50f5004-d01e-4b88-89d3-c95245d36ab2", "C00_Password.png"},
    {"c93c4854-c93d-4b40-8107-6b83b62a2d9c", "C01_Package_Network.png"},
    {"3a6ba8cc-6c49-439a-840a-ef476a7b1a0a", "C02_MessageBox_Warning.png"},
    {"0bbf84a9-ca2a-4eb2-a359-5cdc56bc68f2", "C03_Server.png"},
    {"55fb3c4f-4c49-421b-b7b6-3f7ab97e7488", "C04_Klipper.png"},
    {"1c3c99f0-2714-4ef6-88c6-c0e44116ba50", "C05_Edu_Languages.png"},
    {"a1e4d85c-dc8d-4e66-a106-c610f851167f", "C06_KCMDF.png"},
    {"8afd34b5-fb61-47a6-b800-16415c02884e", "C07_Kate.png"},
    {"c9d08750-c59f-4ee4-be54-3a8da6fad734", "C08_Socket.png"},
    {"e7b3cda9-e1d0-4a3d-a07e-a1df6a37e450", "C09_Identity.png"},
    {"673537ac-987a-4bdf-b57f-b0287f5a0045", "C10_Kontact.png"},
    {"80d0409c-39bb-40c0-a312-a3def2428116", "C11_Camera.png"},
    {"c1e79cee-8460-4a77-be93-8edc35fa2f30", "C12_IRKickFlash.png"},
    {"2b1808f8-a4a4-44bb-b59a-0f2e62de9dda", "C13_KGPG_Key3.png"},
    {"1096fe38-afa7-43dd-9578-89087b702d4e", "C14_Laptop_Power.png"},
    {"da11f5b1-6ab1-49e5-b218-a3d901c049aa", "C15_Scanner.png"},
    {"195698fd-4d3f-4795-98ee-02588b68f429", "C16_Mozilla_Firebird.png"},
    {"daa4e3f7-9f5b-405b-a8ca-f872902b6eed", "C17_CDROM_Unmount.png"},
    {"2c21e109-33b2-48b2-8e41-d6ec2eaae1b2", "C18_Display.png"},
    {"1f578bbe-163e-4baf-afe5-4217e9ebb29a", "C19_Mail_Generic.png"},
    {"a06c1feb-0971-4ae6-a1ac-86bac0ba1aaf", "C20_Misc.png"},
    {"bf5fadf9-994f-441a-a572-465c8ad48325", "C21_KOrganizer.png"},
    {"915b9451-8358-4f17-ad2c-b333c567b634", "C22_ASCII.png"},
    {"27ad947d-9818-4b6e-9105-ececc8c81487", "C23_Icons.png"},
    {"cfd80aa3-eb26-4a75-807a-149f44731244", "C24_Connect_Established.png"},
    {"924e2e73-3eb0-4e58-acf4-3348a96cf750", "C25_Folder_Mail.png"},
    {"b7151e5b-46be-4d00-ba32-835efa6d6ee4", "C26_FileSave.png"},
    {"9d89ba91-988a-4c3f-ba8f-c57fedc6274c", "C27_NFS_Unmount.png"},
    {"b6a21955-a940-41b0-9a4f-a04bd0e444f5", "C28_QuickTime.png"},
    {"a34f5825-5948-47b6-a1bc-b7ca0a710549", "C29_KGPG_Term.png"},
    {"e91f9e68-91ac-4585-9f39-48613e34f937", "C30_Konsole.png"},
    {"aa86ad96-9c9a-4e33-a479-9704ca028536", "C31_FilePrint.png"},
    {"d2000c8b-26c2-469c-a5c2-4a2d41b10cb7", "C32_FSView.png"},
    {"f46e504d-c8e2-45c3-93e5-a1a93a9c4fec", "C33_Run.png"},
    {"13b08c74-1e9c-4996-902a-2ee584ba02ed", "C34_Configure.png"},
    {"c6753ca9-e431-461f-9dc2-b30b4775dd84", "C35_KRFB.png"},
    {"b88936c5-1702-4c5f-bdfe-72dd0ed3c2c9", "C36_Ark.png"},
    {"cc615492-22b4-4eff-b636-e5b6eee308c9", "C37_KPercentage.png"},
    {"5948c41f-f57e-41c7-a06a-e0211bb02d84", "C38_Samba_Unmount.png"},
    {"6b9a01a2-c076-4ed2-b013-b566647418bd", "C39_History.png"},
    {"14aaeba4-d9d7-4c83-b84d-5a951b90ecf9", "C40_Mail_Find.png"},
    {"843c6fb1-14d0-41e7-8903-41e33d4595a0", "C41_VectorGfx.png"},
    {"781a03cc-e043-4067-a971-f4db72ccd626", "C42_KCMMemory.png"},
    {"1ab5edcf-c9cf-4416-9d73-7fee6423e62b", "C43_EditTrash.png"},
    {"c0882dd7-3703-40f8-9bf6-8b3b00f1f775", "C44_KNotes.png"},
    {"1dee76e6-4ea7-42ae-ac60-d2785d9bda11", "C45_Cancel.png"},
    {"7e1cb1fb-7efd-4176-94d4-bc9fd872d855", "C46_Help.png"},
    {"ab977ea6-e36a-4f4a-9ab8-07aef83ed390", "C47_KPackage.png"},
    {"5b265c74-ac61-4be7-af04-809a29ab851d", "C48_Folder.png"},
    {"e80e7a78-c38b-4f01-9480-e44f38c4056f", "C49_Folder_Blue_Open.png"},
    {"e12e4fd5-865e-46b5-8dc4-f55400f56613", "C50_Folder_Tar.png"},
    {"9a7523d6-9f8c-4c9a-b5c6-f604731c7b8a", "C51_Decrypted.png"},
    {"c667e574-81aa-4a44-b7f1-eaadcd656e00", "C52_Encrypted.png"},
    {"c8227374-f8d0-4985-9412-5bd9c7468c53", "C53_Apply.png"},
    {"ae3d33dd-d395-4cba-87e9-8583041398f7", "C54_Signature.png"},
    {"dcd4e840-7fbd-4b82-826a-6190196bc27f", "C55_Thumbnail.png"},
    {"8f2c599f-e57c-4573-ab84-b8ba54f883f8", "C56_KAddressBook.png"},
    {"57989ed9-8d59-4c32-a038-821ad0492af5", "C57_View_Text.png"},
    {"62586f8b-d742-4325-8b12-52f4c05030f3", "C58_KGPG.png"},
    {"de04f667-cae6-40be-b96d-aa048e37c445", "C59_Package_Development.png"},
    {"1df17d3f-83eb-494a-89a6-e69369f960ee", "C60_KFM_Home.png"},
    {"2f08cc69-b14e-4b22-b489-6fb617e60287", "C61_Services.png"},
    {"3078a579-712d-4af4-b3c9-551e9b4c0e3c", "C62_Tux.png"},
    {"1b61c05f-27ec-40a3-9bfa-412d48e64705", "C63_Feather.png"},
    {"513e2e6d-68e8-4ee1-89de-9b2b32f2d144", "C64_Apple.png"},
    {"579ae427-8d7c-4ada-a0df-908c5319efae", "C65_W.png"},
    {"fbd0aec7-734d-431c-84f1-2665f701af6c", "C66_Money.png"},
    {"c0125658-689b-4f0a-8394-f9525d5a633f", "C67_Certificate.png"},
    {"dffe914e-4ee3-4bab-8c7d-b522cb154c2c", "C68_BlackBerry.png"}
};
// clang-format on

QPixmap DatabaseIcons::iconPixmap(QUuid database, QUuid icon)
{
    if (index < 0 || index >= IconCount) {
        qWarning("DatabaseIcons::iconPixmap: invalid icon index %d", index);
        return QPixmap();
    }

    QPixmap pixmap;

    if (!QPixmapCache::find(m_pixmapCacheKeys[index], &pixmap)) {
        pixmap = QPixmap::fromImage(icon(index));
        m_pixmapCacheKeys[index] = QPixmapCache::insert(pixmap);
    }

    return pixmap;
}

DatabaseIcons::DatabaseIcons()
{
    Q_ASSERT(m_commonIcons.size() == 69);


    m_icons.insert(CommonIconsUuid);
}

DatabaseIcons* DatabaseIcons::instance()
{
    if (!m_instance) {
        m_instance = new DatabaseIcons();
    }

    return m_instance;
}
