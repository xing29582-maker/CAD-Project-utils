#pragma once

#include "CommonExport.h"
#include <memory>

namespace cadutils
{
	class CADUTILS_COMMON_API Vector3d
	{
	public:
        Vector3d(double x = 0.0, double y = 0.0, double z = 0.0);
        ~Vector3d();

        void SetX(double x);
        void SetY(double y);
        void SetZ(double z);

        double GetX() const;
        double GetY() const;
        double GetZ() const;

        // ---- 基础长度/归一化 ----
        double Length() const;          // |v|
        double LengthSq() const;        // |v|^2

        // 返回新向量：v.normalized()
        Vector3d Normalized(double eps = 1e-12) const;

        // 原地归一化：v.Normalize(); 返回是否成功（避免0向量）
        bool Normalize(double eps = 1e-12);

        // ---- 点乘/叉乘 ----
        double Dot(const Vector3d& rhs) const;
        Vector3d Cross(const Vector3d& rhs) const;

        // ---- 运算符重载 ----
        Vector3d operator+(const Vector3d& rhs) const;
        Vector3d operator-(const Vector3d& rhs) const;
        Vector3d operator-() const;

        Vector3d& operator+=(const Vector3d& rhs);
        Vector3d& operator-=(const Vector3d& rhs);

        // 标量乘除
        Vector3d operator*(double s) const;
        Vector3d operator/(double s) const;

        Vector3d& operator*=(double s);
        Vector3d& operator/=(double s);

        // 点乘：v * w -> double（很多人喜欢这样写）
        double operator*(const Vector3d& rhs) const;

        // 叉乘：v ^ w -> Vector3d（可选，看你团队习惯）
        Vector3d operator^(const Vector3d& rhs) const;

        // 支持：s * v
        friend Vector3d operator*(double s, const Vector3d& v)
        {
            return v * s;
        }
    private:
        double m_x;
        double m_y;
        double m_z;
	};
}