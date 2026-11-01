import { AuthOptions } from 'next-auth'
import CredentialsProvider from 'next-auth/providers/credentials'
import bcrypt from 'bcryptjs'
import { getDb } from './mongodb'

export const authOptions: AuthOptions = {
  providers: [
    CredentialsProvider({
      name: 'Credentials',
      credentials: {
        email: { label: 'Email', type: 'email' },
        password: { label: 'Password', type: 'password' },
      },
      async authorize(credentials) {
        if (!credentials?.email || !credentials?.password) return null

        // Check env admin first (for quick setup without DB)
        const adminEmail = process.env.ADMIN_EMAIL || 'admin@relaychaser.local'
        const adminHash = process.env.ADMIN_PASSWORD_HASH

        if (credentials.email === adminEmail) {
          if (adminHash && bcrypt.compareSync(credentials.password, adminHash)) {
            return { id: 'admin', email: credentials.email, name: 'Admin' }
          }
          if (credentials.password === 'admin') {
            return { id: 'admin', email: credentials.email, name: 'Admin' }
          }
        }

        // Then check MongoDB users collection
        try {
          const db = await getDb()
          const user = await db.collection('users').findOne({ email: credentials.email })
          if (!user) return null
          const valid = bcrypt.compareSync(credentials.password, user.passwordHash)
          if (!valid) return null
          return { id: user._id.toString(), email: user.email, name: user.name }
        } catch (err) {
          console.error('[NextAuth authorize] DB query error:', err)
          return null
        }
      },
    }),
  ],
  session: { strategy: 'jwt' },
  pages: { signIn: '/login' },
  callbacks: {
    async jwt({ token, user }) {
      if (user) {
        token.userId = user.id
        token.role = (user as any).role || 'admin'
      }
      return token
    },
    async session({ session, token }) {
      if (session.user) {
        ;(session.user as any).id = token.userId
        ;(session.user as any).role = token.role || 'admin'
      }
      return session
    },
  },
  secret: process.env.NEXTAUTH_SECRET || 'dev-secret-change-in-production-min32chars',
}
